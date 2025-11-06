#include "can.h"

int CanInit(CAN_HandleTypeDef* hcan)
{
    CAN_FilterTypeDef f = {0};
    int status = HAL_OK;

    // Configure filter to accept all messages
    f.FilterBank = 0;
    f.FilterMode = CAN_FILTERMODE_IDMASK;
    f.FilterScale = CAN_FILTERSCALE_32BIT;
    f.FilterIdHigh = 0x0000;
    f.FilterIdLow = 0x0000;
    f.FilterMaskIdHigh = 0x0000;
    f.FilterMaskIdLow = 0x0000;
    f.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    f.FilterActivation = CAN_FILTER_ENABLE;
    f.SlaveStartFilterBank = 14;

    if ((status = HAL_CAN_ConfigFilter(hcan, &f)) != HAL_OK)
        return status;

    if ((status = HAL_CAN_Start(hcan)) != HAL_OK)
        return status;

    if ((status = HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING)) != HAL_OK)
        return status;

    return HAL_OK;
}

int CanTransmit(CAN_HandleTypeDef* hcan, uint32_t id, uint8_t data[8])
{
    CAN_TxHeaderTypeDef hdr = {0};
    uint32_t mailbox;

    hdr.StdId = id;
    hdr.IDE = CAN_ID_STD;
    hdr.RTR = CAN_RTR_DATA;
    hdr.DLC = 8;
    hdr.TransmitGlobalTime = DISABLE;

    uint32_t timeout = 1000; // ~1ms
    while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0)
    {
        if (--timeout == 0)
            return HAL_TIMEOUT;
    }

    return HAL_CAN_AddTxMessage(hcan, &hdr, data, &mailbox);
}

int CanTxHeartbeat(SystemState* state)
{
    uint8_t frame[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    return CanTransmit(state->hw.can, CANID_TX_HEARTBEAT, frame);
}

int ConfigCan(SystemState* state)
{
    return CanInit(state->hw.can);
}