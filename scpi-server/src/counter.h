/**
 * $Id: $
 *
 * @brief Red Pitaya counter module SCPI interface
 *
 * @Author Roger John, roger.john@uni-leipzig.de / Robert Staacke, robert.staacke@physik.uni-leipzig.de
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef COUNTER_H
#define COUNTER_H
#include "scpi/types.h"

scpi_result_t RP_CounterSendCmd(scpi_t * context);
scpi_result_t RP_CounterGetState(scpi_t * context);
scpi_result_t RP_CounterSetCountingTime(scpi_t * context);
scpi_result_t RP_CounterGetCountingTime(scpi_t * context);
scpi_result_t RP_CounterGetCounts(scpi_t * context);
scpi_result_t RP_CounterSetNumberOfBins(scpi_t * context);
scpi_result_t RP_CounterGetNumberOfBins(scpi_t * context);
scpi_result_t RP_CounterSetRepetitions(scpi_t * context);
scpi_result_t RP_CounterGetRepetitions(scpi_t * context);
scpi_result_t RP_CounterSetPredelay(scpi_t * context);
scpi_result_t RP_CounterGetPredelay(scpi_t * context);
scpi_result_t RP_CounterSetTriggerConfig(scpi_t * context);
scpi_result_t RP_CounterGetTriggerConfig(scpi_t * context);
scpi_result_t RP_CounterSetBinsSplitted(scpi_t * context);
scpi_result_t RP_CounterGetBinsSplitted(scpi_t * context);
scpi_result_t RP_CounterSetGatedCounting(scpi_t * context);
scpi_result_t RP_CounterGetGatedCounting(scpi_t * context);
scpi_result_t RP_CounterGetBinAddress(scpi_t * context);
scpi_result_t RP_CounterGetRepetitionCounter(scpi_t * context);
scpi_result_t RP_CounterGetBinData(scpi_t * context);
scpi_result_t RP_CounterResetBinData(scpi_t * context);
scpi_result_t RP_CounterResetBinDataPartially(scpi_t * context);
scpi_result_t RP_CounterReset(scpi_t * context);
scpi_result_t RP_CounterCount(scpi_t * context);
scpi_result_t RP_CounterCountSingle(scpi_t * context);
scpi_result_t RP_CounterGetTriggeredCounting(scpi_t * context);
scpi_result_t RP_CounterSetTriggeredCounting(scpi_t * context);
scpi_result_t RP_CounterTrigger(scpi_t * context);
scpi_result_t RP_CounterGetNumCounters(scpi_t * context);
scpi_result_t RP_CounterGetMaxBins(scpi_t * context);
scpi_result_t RP_CounterGetDNA(scpi_t * context);
scpi_result_t RP_CounterGetClock(scpi_t * context);
scpi_result_t RP_CounterWaitForState(scpi_t * context);
scpi_result_t RP_CounterWaitAndReadAndStartCounting(scpi_t * context);
scpi_result_t RP_CounterStartAnalogOutput(scpi_t * context);
scpi_result_t RP_CounterStopAnalogOutput(scpi_t * context);

#endif /* GENERATE_H_ */
