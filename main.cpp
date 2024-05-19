#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include "okFrontPanel.h"
#include "rhxcontroller.h"
#include "rhxregisters.h"
#include "rhxdatablock.h"

int main() {

    const int STREAM = 0;
    const int CHANNEL = 0;

    //NOTE: Each stream can only have 16 channels, sop even for a single RHS2116 with 32 channels, we need to setup 2 streams
    ///////////////////////////////////////////////////////////
    /////////////////////INIT SEQUENCE STARTS//////////////////
    ///////////////////////////////////////////////////////////

    // Create RHX Controller with 30 kHz per-amplifier sampling rate.
    RHXController *rhxController = new RHXController(ControllerStimRecord, SampleRate30000Hz, true);

    // Open the first detected Opal Kelly device, load RhythmStim USB-7310 bitfile.
    vector<string> availableDevices = rhxController->listAvailableDeviceSerials();
    rhxController->open(availableDevices[0]);

    // Load RhythmStim USB-7310 bitfile and initialize.
    rhxController->uploadFPGABitfile("ConfigRHSController_7310.bit");
    rhxController->initialize();
    // for (int i = 0; i < 8; i++){
    //     rhxController->enableDataStream(i, true);
    // }
    rhxController->enableDataStream(STREAM, true);

    if (rhxController->isSynthetic() || rhxController->isPlayback()){
        std::cout << "A REAL XEM7310 IS NOT CONNECTED, DISCONNECTING" ;
        return 0;
    }
	 
    //checking for the sample rate
    if (rhxController->getSampleRate() != 30000){
        std::cout << "sampleRate is not 30,000!\n";
    }
    
    // We can set the MISO sampling delay which is dependent on the sample rate.  We assume a 3-foot cable.
    rhxController->setCableLengthFeet(PortA, 3.0);

    // Let's turn one LED on to indicate that the program is running.
    int ledArray[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    rhxController->setLedDisplay(ledArray);

    //[VERIFIED: THE CODE TILL ABOVE HAS BEEN VERIFIED]

    ///////////////////////////////////////////////////////////
    /////////////////////INIT SEQUENCE ENDS////////////////////
    ///////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////
    ///////////////REGS SEQUENCE STARTS////////////////////////
    ///////////////////////////////////////////////////////////

    //std::cout << rhxController->getNumEnabledDataStreams() << "\n";

    RHXRegisters *chipRegisters = new RHXRegisters(rhxController->getType(), rhxController->getSampleRate(), StimStepSize10nA);

    chipRegisters->setStimEnable(true);
    chipRegisters->setStimStepSize(StimStepSize10nA);
    chipRegisters->setPosStimMagnitude(CHANNEL, 200, 0);
    chipRegisters->setNegStimMagnitude(CHANNEL, 200, 0);
    rhxController->setContinuousRunMode(true);
    rhxController->run();

    int commandSequenceLength;
    vector<unsigned int> commandList;
    commandSequenceLength = chipRegisters->createCommandListRHSRegisterConfig(commandList, true);
    rhxController->uploadCommandList(commandList, AbstractRHXController::AuxCmd1);
    rhxController->selectAuxCommandLength(AbstractRHXController::AuxCmd1, 0, commandSequenceLength - 1);
    chipRegisters->createCommandListDummy(commandList, 8192, chipRegisters->createRHXCommand(RHXRegisters::RHXCommandRegRead, 255));
    rhxController->uploadCommandList(commandList, AbstractRHXController::AuxCmd2);
    rhxController->uploadCommandList(commandList, AbstractRHXController::AuxCmd3);
    rhxController->uploadCommandList(commandList, AbstractRHXController::AuxCmd4);
    rhxController->enableAuxCommandsOnAllStreams();

    ///////////////////////////////////////////////////////////
    ///////////////REGS SEQUENCE ENDS////////////////////////
    ///////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////
    ///////////////CHECKING FOR STIM STARTS////////////////////
    ///////////////////////////////////////////////////////////

    RHXDataBlock *dataBlock = new RHXDataBlock(rhxController->getType(), rhxController->getNumEnabledDataStreams());
    rhxController->readDataBlock(dataBlock);

    do {
        RHXDataBlock *dataBlock = new RHXDataBlock(rhxController->getType(), rhxController->getNumEnabledDataStreams());
        rhxController->readDataBlock(dataBlock);
        // Display register contents from data stream 0.
        dataBlock->print(0);
    } while (rhxController->isRunning());

    ///////////////////////////////////////////////////////////
    ///////////////CHECKING FOR STIM ENDS//////////////////////
    ///////////////////////////////////////////////////////////
        
    ///////////////////////////////////////////////////////////
    ////////////CLEAN UP STARTS ///////////////////////////////
    ///////////////////////////////////////////////////////////


    // Turn off LED.
    ledArray[0] = 0;
    rhxController->setLedDisplay(ledArray);

    delete chipRegisters;
    delete rhxController;

    ///////////////////////////////////////////////////////////
    ////////////CLEAN UP ENDS ///////////////////////////////
    ///////////////////////////////////////////////////////////
    return 0;
}
