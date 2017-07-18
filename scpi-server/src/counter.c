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

#include <stdlib.h>
#include <string.h>
#include "counter.h"
#include "../../api/include/redpitaya/rp.h"

#include "common.h"
#include "scpi/parser.h"

/* cmd choice def */
const scpi_choice_def_t scpi_RpCounterCmd[] = {
    {"NONE", RP_none},
    {"IDLE", RP_gotoIdle},
    {"RESET", RP_reset},
    {"COUNT", RP_countImmediately },
    {"TRIGCOUNT",  RP_countTriggered},
    {"TRIGger",  RP_trigger},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_RpCounterStates[] = {
	{"idle",RP_idle},
	{"immediateCountingStart",RP_immediateCountingStart},
	{"immediateCountingWaitForTimeout",RP_immediateCountingWaitForTimeout},
	{"triggeredCountingWaitForTrigger",RP_triggeredCountingWaitForTrigger},
	{"triggeredCountingStore",RP_triggeredCountingStore},
	{"triggeredCountingPredelay",RP_triggeredCountingPredelay},
	{"triggeredCountingPrestore",RP_triggeredCountingPrestore},
	{"triggeredCountingWaitForTimeout",RP_triggeredCountingWaitForTimeout},
	SCPI_CHOICE_LIST_END
};

scpi_result_t RP_CounterSendCmd(scpi_t *context) {
    rp_counterCmd_t cmd;
    if (!SCPI_ParamChoice(context, scpi_RpCounterCmd, (int32_t*)&cmd, true)) {
        RP_LOG(LOG_ERR, "*Invalid Command.\n");
        return SCPI_RES_ERR;
    }
    int result = rp_CounterSendCmd(cmd);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterGetState(scpi_t *context) {
    rp_counterState_t state;
    int result = rp_CounterGetState(&state);
    if (RP_OK != result) {/*place holder for Error*/
        RP_LOG(LOG_ERR, "*unknown error.\n");
        return SCPI_RES_ERR;
    }
    if (!(RP_idle<=state && state<=RP_triggeredCountingWaitForTimeout)) {/*place holder for Error*/
        RP_LOG(LOG_ERR, "*unknown state.\n");
        return SCPI_RES_ERR;
    }
    SCPI_ResultText(context,scpi_RpCounterStates[state].name);
    RP_LOG(LOG_INFO, "state successfully retrieved");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterSetCountingTime(scpi_t *context) {
    float countingTime;
    if (!SCPI_ParamFloat(context, &countingTime, true)) {
        RP_LOG(LOG_ERR, "*COUNT:TIME is missing parameter.\n");
        return SCPI_RES_ERR;
    }
    int result = rp_CounterSetCountingTime(countingTime);
    if (RP_OK != result) {/*place holder for Error*/
		RP_LOG(LOG_ERR, "*unknown error.\n");
		return SCPI_RES_ERR;
	}

    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterGetCountingTime(scpi_t *context) {
    float countingTime;
    int result = rp_CounterGetCountingTime(&countingTime);
    if (RP_OK != result) {/*place holder for Error*/
        RP_LOG(LOG_ERR, "*unknown error.\n");
        return SCPI_RES_ERR;
    }
    SCPI_ResultFloat(context,countingTime);
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterGetCounts(scpi_t *context) {
    uint32_t counts[RP_COUNTER_NUM_COUNTERS];
    int result = rp_CounterGetCounts(counts);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    for(int i=0;i<RP_COUNTER_NUM_COUNTERS;i++)
    	SCPI_ResultUInt32Base(context,counts[i],10);
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterSetNumberOfBins(scpi_t *context) {
    uint32_t numBins;
    if (!SCPI_ParamUInt32(context, &numBins, true)) {
        RP_LOG(LOG_ERR, "*COUNT:BINS:NO is missing parameter.\n");
        return SCPI_RES_ERR;
    }
    int result = rp_CounterSetNumberOfBins(numBins);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterGetNumberOfBins(scpi_t *context) {
    uint32_t numBins;
    int result = rp_CounterGetNumberOfBins(&numBins);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    SCPI_ResultUInt32Base(context,numBins,10);
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterSetRepetitions(scpi_t *context) {
    uint32_t repetitions;
    if (!SCPI_ParamUInt32(context, &repetitions, true)) {
        RP_LOG(LOG_ERR, "*COUNT:REPT is missing parameter.\n");
        return SCPI_RES_ERR;
    }
    int result = rp_CounterSetRepetitions(repetitions);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterGetRepetitions(scpi_t *context) {
    uint32_t repetitions;
    int result = rp_CounterGetRepetitions(&repetitions);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    SCPI_ResultUInt32Base(context,repetitions,10);
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterSetPredelay(scpi_t *context) {
    float preDelay;
    if (!SCPI_ParamFloat(context, &preDelay, true)) {
        RP_LOG(LOG_ERR, "*COUNT:DELAY is missing parameter.\n");
        return SCPI_RES_ERR;
    }
    int result = rp_CounterSetPredelay(preDelay);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterGetPredelay(scpi_t *context) {
    float preDelay;
    int result = rp_CounterGetPredelay(&preDelay);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    SCPI_ResultFloat(context,preDelay);
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterSetTriggerConfig(scpi_t *context) {
	/*triggerMask=Bit Mask:
	    1 -> 0001 input channel pin 1
	    2 -> 0010 input channel pin 2
	    4 -> 0100 input channel pin 3
	    8 -> 1000 input channel pin 4
	  Invert:
	    inverts input if 1
	  Polarity (makes only sense for more than one inputs):
	 	inverts inputs after triggerInvert + "or" over all trigger inputs
	*/
    uint32_t triggerMask,triggerInvert,triggerPolarity;
    if (!SCPI_ParamUInt32(context, &triggerMask, true)
            ||!SCPI_ParamUInt32(context, &triggerInvert, true)
            ||!SCPI_ParamUInt32(context, &triggerPolarity, true)) {
        RP_LOG(LOG_ERR, "*COUNT:TRIGGERCONF is missing parameter.\n");
        return SCPI_RES_ERR;
    }
    int result = rp_CounterSetTriggerConfig(triggerMask,triggerInvert,triggerPolarity!=0);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterGetTriggerConfig(scpi_t *context) {
    uint32_t triggerMask,triggerInvert;
    bool triggerPolarity;
    int result = rp_CounterGetTriggerConfig(&triggerMask,&triggerInvert,&triggerPolarity);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    SCPI_ResultUInt32Base(context,triggerMask,10);
    SCPI_ResultUInt32Base(context,triggerInvert,10);
    SCPI_ResultUInt32Base(context,triggerPolarity?1:0,10);
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterSetBinsSplitted(scpi_t *context) {
    scpi_bool_t binsSplitted;
    if (!SCPI_ParamBool(context, &binsSplitted, true)) {
        RP_LOG(LOG_ERR, "*COUNT:BINS:SPLIT is missing parameter.\n");
        return SCPI_RES_ERR;
    }
    int result = rp_CounterSetBinsSplitted(binsSplitted);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterGetBinsSplitted(scpi_t *context) {
    scpi_bool_t binsSplitted;
    int result = rp_CounterGetBinsSplitted(&binsSplitted);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    SCPI_ResultBool(context,binsSplitted);
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterSetGatedCounting(scpi_t *context) {
    scpi_bool_t gatedCounting;
    if (!SCPI_ParamBool(context, &gatedCounting, true)) {
        RP_LOG(LOG_ERR, "*COUNT:GATED is missing parameter.\n");
        return SCPI_RES_ERR;
    }
    int result = rp_CounterSetGatedCounting(gatedCounting);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterGetGatedCounting(scpi_t *context) {
    scpi_bool_t gatedCounting;
    int result = rp_CounterGetGatedCounting(&gatedCounting);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    SCPI_ResultBool(context,gatedCounting);
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterGetBinAddress(scpi_t *context) {
    uint32_t binAddress;
    int result = rp_CounterGetBinAddress(&binAddress);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    SCPI_ResultUInt32Base(context,binAddress,10);
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterGetRepetitionCounter(scpi_t *context) {
    uint32_t repetitionCounter;
    int result = rp_CounterGetRepetitionCounter(&repetitionCounter);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    SCPI_ResultUInt32Base(context,repetitionCounter,10);
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}
scpi_result_t RP_CounterGetBinData(scpi_t *context) {
    uint32_t numBins;
    if (!SCPI_ParamUInt32(context, &numBins, true)) {
        RP_LOG(LOG_ERR, "*COUNT:BINS:DATA is missing parameter.\n");
        return SCPI_RES_ERR;
    }
    if(numBins>RP_COUNTER_BINS) {
        RP_LOG(LOG_ERR, "*COUNT:BINS:DATA called for too many bins.\n");
        return SCPI_RES_ERR;
    }
    uint32_t *binData[RP_COUNTER_NUM_COUNTERS];
    for(int i=0;i<RP_COUNTER_NUM_COUNTERS;i++)
        binData[i] = malloc(sizeof(uint32_t)*numBins);
    int result = rp_CounterGetBinData(binData,numBins);
    if (RP_OK != result) {/*place holder for Error*/
    	for (int i=0;i<RP_COUNTER_NUM_COUNTERS;i++)
    	  	free(binData[i]);
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    for (int i=0;i<RP_COUNTER_NUM_COUNTERS;i++)
    	for (int j=0;j<numBins;j++)
    		SCPI_ResultUInt32Base(context,binData[i][j],10);
    for (int i=0;i<RP_COUNTER_NUM_COUNTERS;i++)
    	free(binData[i]);
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}

scpi_result_t RP_CounterResetBinData(scpi_t *context) {
    int result = rp_CounterResetBinData();
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}

scpi_result_t RP_CounterResetBinDataPartially(scpi_t *context) {
    uint32_t numBins;
    if (!SCPI_ParamUInt32(context, &numBins, true)) {
        RP_LOG(LOG_ERR, "*COUNT:BINS:RESET is missing parameter.\n");
        return SCPI_RES_ERR;
    }
    if(numBins>RP_COUNTER_BINS) {
        RP_LOG(LOG_ERR, "*COUNT:BINS:RESET called for too many bins.\n");
        return SCPI_RES_ERR;
    }
    int result = rp_CounterResetBinDataPartially(numBins);
    if (RP_OK != result) {/*place holder for Error*/
        RP_LOG(LOG_ERR, "*unknown error.\n");
        return SCPI_RES_ERR;
    }
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}

scpi_result_t RP_CounterCount(scpi_t *context) {
    uint32_t numCounts;
    if (!SCPI_ParamUInt32(context, &numCounts, true)) {
        RP_LOG(LOG_ERR, "*COUNT:COUNT is missing parameter.\n");
        return SCPI_RES_ERR;
    }
    uint32_t *counts[RP_COUNTER_NUM_COUNTERS];
    for(int i=0;i<RP_COUNTER_NUM_COUNTERS;i++)
        counts[i] = malloc(sizeof(uint32_t)*numCounts);
    int result = rp_CounterCount(counts,numCounts);
    if (RP_OK != result) {/*place holder for Error*/
        for (int i=0;i<RP_COUNTER_NUM_COUNTERS;i++)
            free(counts[i]);
        RP_LOG(LOG_ERR, "*unknown error.\n");
        return SCPI_RES_ERR;
    }
    for (int i=0;i<RP_COUNTER_NUM_COUNTERS;i++)
        for (int j=0;j<numCounts;j++)
            SCPI_ResultUInt32Base(context,counts[i][j],10);
    for (int i=0;i<RP_COUNTER_NUM_COUNTERS;i++)
        free(counts[i]);    
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}

scpi_result_t RP_CounterCountSingle(scpi_t *context) {
    uint32_t counts[RP_COUNTER_NUM_COUNTERS];
    int result = rp_CounterCountSingle(counts);
    if (RP_OK != result) {/*place holder for Error*/
        RP_LOG(LOG_ERR, "*unknown error.\n");
        return SCPI_RES_ERR;
    }
    for (int i=0;i<RP_COUNTER_NUM_COUNTERS;i++)
        SCPI_ResultUInt32Base(context,counts[i],10);
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}

scpi_result_t RP_CounterSetTriggeredCounting(scpi_t *context) {
    scpi_bool_t triggeredCounting;
    if (!SCPI_ParamBool(context, &triggeredCounting, true)) {
        RP_LOG(LOG_ERR, "*COUNT:TriGGERED is missing parameter.\n");
        return SCPI_RES_ERR;
    }
    int result = rp_CounterSetTriggeredCounting(triggeredCounting);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}

scpi_result_t RP_CounterGetTriggeredCounting(scpi_t *context) {
    scpi_bool_t triggeredCounting;
    int result = rp_CounterGetTriggeredCounting(&triggeredCounting);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    SCPI_ResultBool(context,triggeredCounting);
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}

scpi_result_t RP_CounterTrigger(scpi_t * context) {
    int result = rp_CounterTrigger();
    if (RP_OK != result) {/*place holder for Error*/
        RP_LOG(LOG_ERR, "*unknown error.\n");
        return SCPI_RES_ERR;
    }
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;

}

scpi_result_t RP_CounterGetNumCounters(scpi_t * context) {
    SCPI_ResultUInt32Base(context,RP_COUNTER_NUM_COUNTERS,10);
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}

scpi_result_t RP_CounterGetMaxBins(scpi_t * context) {
    SCPI_ResultUInt32Base(context,RP_COUNTER_BINS,10);
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}

scpi_result_t RP_CounterReset(scpi_t * context) {
    int result = rp_CounterReset();
    if (RP_OK != result) {/*place holder for Error*/
        RP_LOG(LOG_ERR, "*unknown error.\n");
        return SCPI_RES_ERR;
    }
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}

scpi_result_t RP_CounterGetDNA(scpi_t * context) {
    uint32_t dna;
    int result = rp_CounterGetDNA(&dna);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    SCPI_ResultUInt32Base(context,dna,16);
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}

scpi_result_t RP_CounterGetClock(scpi_t * context) {
    uint32_t clock;
    int result = rp_CounterGetClock(&clock);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    SCPI_ResultUInt32Base(context,clock,10);
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}

scpi_result_t RP_CounterWaitForState(scpi_t * context) {
    rp_counterState_t state;
    if (!SCPI_ParamChoice(context, scpi_RpCounterStates, (int32_t*)&state, true)) {
        RP_LOG(LOG_ERR, "*Invalid state.\n");
        return SCPI_RES_ERR;
    }
    int result = rp_CounterWaitForState(state);
    if (RP_OK != result) {/*place holder for Error*/
    	RP_LOG(LOG_ERR, "*unknown error.\n");
    	return SCPI_RES_ERR;
    }
    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}

scpi_result_t RP_CounterWaitAndReadAndStartCounting(scpi_t * context) {
	uint32_t counts[RP_COUNTER_NUM_COUNTERS];
	int result = rp_CounterWaitAndReadAndStartCounting(counts);
	if (RP_OK != result) {/*place holder for Error*/
		RP_LOG(LOG_ERR, "*unknown error.\n");
		return SCPI_RES_ERR;
	}
	for (int i = 0; i < RP_COUNTER_NUM_COUNTERS; i++)
		SCPI_ResultUInt32Base(context, counts[i], 10);
	RP_LOG(LOG_INFO, "Command successfully send");
	return SCPI_RES_OK;
}

scpi_result_t RP_CounterStartAnalogOutput(scpi_t *context) {
    uint32_t MaxCount=1e6, MinCount = 0;
    if (!SCPI_ParamUInt32(context, &MaxCount, false)
    		||!SCPI_ParamUInt32(context, &MinCount, false)) {
        RP_LOG(LOG_ERR, "*COUNT:TIME is missing parameter.\n");
        return SCPI_RES_ERR;
    }
    int result = rp_CounterStartAnalogOutput(MaxCount,MinCount);
    if (RP_OK != result) {/*place holder for Error*/
		RP_LOG(LOG_ERR, "*unknown error.\n");
		return SCPI_RES_ERR;
	}

    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}

scpi_result_t RP_CounterStopAnalogOutput(scpi_t *context) {
    int result = rp_CounterStopAnalogOutput();
    if (RP_OK != result) {/*place holder for Error*/
		RP_LOG(LOG_ERR, "*unknown error.\n");
		return SCPI_RES_ERR;
	}

    RP_LOG(LOG_INFO, "Command successfully send");
    return SCPI_RES_OK;
}

