#include "enet.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*! @brief Buffer descriptors should be in non-cacheable region and should be align to "ENET_BUFF_ALIGNMENT". */
AT_NONCACHEABLE_SECTION_ALIGN(enet_rx_bd_struct_t g_rxBuffDescrip[ENET_RXBD_NUM], ENET_BUFF_ALIGNMENT);
AT_NONCACHEABLE_SECTION_ALIGN(enet_tx_bd_struct_t g_txBuffDescrip[ENET_TXBD_NUM], ENET_BUFF_ALIGNMENT);
/*! @brief The data buffers can be in cacheable region or in non-cacheable region.
 * If use cacheable region, the alignment size should be the maximum size of "CACHE LINE SIZE" and "ENET_BUFF_ALIGNMENT"
 * If use non-cache region, the alignment size is the "ENET_BUFF_ALIGNMENT".
 */
SDK_ALIGN(uint8_t g_rxDataBuff[ENET_RXBD_NUM][SDK_SIZEALIGN(ENET_RXBUFF_SIZE, APP_ENET_BUFF_ALIGNMENT)],
          APP_ENET_BUFF_ALIGNMENT);
SDK_ALIGN(uint8_t g_txDataBuff[ENET_TXBD_NUM][SDK_SIZEALIGN(ENET_TXBUFF_SIZE, APP_ENET_BUFF_ALIGNMENT)],
          APP_ENET_BUFF_ALIGNMENT);

enet_handle_t g_handle;
uint8_t g_frame[ENET_DATA_LENGTH + 14];

/*! @brief The MAC address for ENET device. */
uint8_t g_macAddr[6] = MAC_ADDRESS;

/*! @brief Enet PHY and MDIO interface handler. */
static mdio_handle_t mdioHandle = {.ops = &EXAMPLE_MDIO_OPS};
static phy_handle_t phyHandle   = {.phyAddr = EXAMPLE_PHY_ADDRESS, .mdioHandle = &mdioHandle, .ops = &EXAMPLE_PHY_OPS};

/*******************************************************************************
 * Code
 ******************************************************************************/

enet_config_t config;
phy_config_t phyConfig = {0};
uint32_t length        = 0;
bool link              = false;
bool autonego          = false;
phy_speed_t speed;
phy_duplex_t duplex;
uint32_t testTxNum = 0;
status_t status;
enet_data_error_stats_t eErrStatic;
volatile uint32_t count = 0;

void transmit_frame(uint8_t *cypher_message, size_t length)
{
	size_t frame_len = length + 14;

    //dest. addr
    g_frame[0] = 0x68U;
    g_frame[1] = 0xE4U;
	g_frame[2] = 0x3BU;
	g_frame[3] = 0x30U;
	g_frame[4] = 0x63U;
	g_frame[5] = 0x9AU;

    //src. addr
    memcpy(&g_frame[6], &g_macAddr[0], 6U);

    //etherType
    g_frame[12] = (length >> 8) & 0xFFU;
    g_frame[13] = length & 0xFFU;

    //message
    memcpy(&g_frame[14], cypher_message, length);

    //padding
    while(frame_len < 64)
    {
    	g_frame[frame_len++] = 0x00;
    }

    if (testTxNum < ENET_TRANSMIT_DATA_NUM)
    {
    	            /* Send a multicast frame when the PHY is link up. */
    	if (kStatus_Success == PHY_GetLinkStatus(&phyHandle, &link))
    	{
    	    if (link)
    	    {
    	        testTxNum++;
    	        if (kStatus_Success ==
    	            ENET_SendFrame(EXAMPLE_ENET, &g_handle, &g_frame[0], frame_len, 0, false, NULL))
    	        {
    	            PRINTF("The %d frame transmitted success!\r\n", testTxNum);
    	        }
    	        else
    	        {
    	            PRINTF(" \r\nTransmit frame failed!\r\n");
    	        }
    	    }
    	}
    }
}

void init_ENET(void)
{
	/* Disable SYSMPU. */
	SYSMPU_Enable(SYSMPU, false);
	    /* Set RMII clock src. */
	CLOCK_SetRmii0Clock(1U);

	PRINTF("\r\nENET example start.\r\n");

	    /* Prepare the buffer configuration. */
	enet_buffer_config_t buffConfig[] =
	{{
		ENET_RXBD_NUM,
	    ENET_TXBD_NUM,
	    SDK_SIZEALIGN(ENET_RXBUFF_SIZE, APP_ENET_BUFF_ALIGNMENT),
	    SDK_SIZEALIGN(ENET_TXBUFF_SIZE, APP_ENET_BUFF_ALIGNMENT),
	    &g_rxBuffDescrip[0],
	    &g_txBuffDescrip[0],
	    &g_rxDataBuff[0][0],
	    &g_txDataBuff[0][0],
	    true,
	    true,
	    NULL,
	}};

	    /* Get default configuration. */
	    /*
	     * config.miiMode = kENET_RmiiMode;
	     * config.miiSpeed = kENET_MiiSpeed100M;
	     * config.miiDuplex = kENET_MiiFullDuplex;
	     * config.rxMaxFrameLen = ENET_FRAME_MAX_FRAMELEN;
	     */
	ENET_GetDefaultConfig(&config);

	    /* The miiMode should be set according to the different PHY interfaces. */
#ifdef EXAMPLE_PHY_INTERFACE_RGMII
	config.miiMode = kENET_RgmiiMode;
#else
	config.miiMode = kENET_RmiiMode;
#endif
	phyConfig.phyAddr               = EXAMPLE_PHY_ADDRESS;
	phyConfig.autoNeg               = true;
	mdioHandle.resource.base        = EXAMPLE_ENET;
	mdioHandle.resource.csrClock_Hz = EXAMPLE_CLOCK_FREQ;

	    /* Initialize PHY and wait auto-negotiation over. */
	PRINTF("Wait for PHY init...\r\n");
	do
	{
	    status = PHY_Init(&phyHandle, &phyConfig);
	    if (status == kStatus_Success)
	    {
	        PRINTF("Wait for PHY link up...\r\n");
	            /* Wait for auto-negotiation success and link up */
	        count = PHY_AUTONEGO_TIMEOUT_COUNT;
	        do
	        {
	            PHY_GetAutoNegotiationStatus(&phyHandle, &autonego);
	            PHY_GetLinkStatus(&phyHandle, &link);
	            if (autonego && link)
	            {
	                break;
	            }
	        } while (--count);
	        if (!autonego)
	        {
	            PRINTF("PHY Auto-negotiation failed. Please check the cable connection and link partner setting.\r\n");
	        }
	    }
	} while (!(link && autonego));

#if PHY_STABILITY_DELAY_US
	    /* Wait a moment for PHY status to be stable. */
	SDK_DelayAtLeastUs(PHY_STABILITY_DELAY_US, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
#endif

	    /* Get the actual PHY link speed. */
	PHY_GetLinkSpeedDuplex(&phyHandle, &speed, &duplex);
	    /* Change the MII speed and duplex for actual link status. */
	config.miiSpeed  = (enet_mii_speed_t)speed;
	config.miiDuplex = (enet_mii_duplex_t)duplex;

	ENET_Init(EXAMPLE_ENET, &g_handle, &config, &buffConfig[0], &g_macAddr[0], EXAMPLE_CLOCK_FREQ);
	ENET_ActiveRead(EXAMPLE_ENET);

}

bool receive_frame(void)
{
	status = ENET_GetRxFrameSize(&g_handle, &length, 0);
	if (length != 0)
	{
	    uint8_t *data = (uint8_t *)malloc(length);
	    status = ENET_ReadFrame(EXAMPLE_ENET, &g_handle, data, length, 0, NULL);
	    if (status == kStatus_Success)
	    {
	    	// Filtro MAC de destino
	    	uint8_t my_mac[6] = {0xD4, 0xBE, 0xD9, 0x45, 0x22, 0x60};

	    	if (memcmp(data, my_mac, 6) != 0)
	    	{
	    	    free(data);
	    	    return false;
	    	}

	    	PRINTF("Frame received. Length: %d ", length);
	        PRINTF("Dst: %02x:%02x:%02x:%02x:%02x:%02x ",
	               data[0], data[1], data[2], data[3], data[4], data[5]);
	        PRINTF("Src: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
	               data[6], data[7], data[8], data[9], data[10], data[11]);

	        // Extraer payload y desencriptar
	        uint8_t *payload = data + 14;
	        uint32_t payload_len = length - 14;

	        // Filtro payload
	        if (payload_len < 16 || (payload_len % 16 != 0))
	        {
	            free(data);
	            return false;
	        }

	        uint8_t response[128];
	        size_t res_len = decrypt_message(payload, payload_len, response);
	        if (res_len > 0)
	        {
	            PRINTF("Respuesta: %s\r\n", response);
	        }

	        free(data);
	        return true;
	    }
	    free(data);
	}
	else if (status == kStatus_ENET_RxFrameError)
	{
	    ENET_GetRxErrBeforeReadFrame(&g_handle, &eErrStatic, 0);
	    ENET_ReadFrame(EXAMPLE_ENET, &g_handle, NULL, 0, 0, NULL);

	}
	return false;
}
