#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include "okFrontPanel.h"
#include "rhxcontroller.h"
#include "rhxregisters.h"
#include "rhxdatablock.h"

int main() {
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
    rhxController->enableDataStream(0, true);

    // We can set the MISO sampling delay which is dependent on the sample rate.  We assume a 3-foot cable.
    rhxController->setCableLengthFeet(PortA, 3.0); //[TO SET]

    // Let's turn one LED on to indicate that the program is running.
    int ledArray[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    rhxController->setLedDisplay(ledArray);

    //[VERIFICATION]: If the LED lights up, the Initilization was successful

    ///////////////////////////////////////////////////////////
    /////////////////////INIT SEQUENCE ENDS////////////////////
    ///////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////
    ////////////REGISTER INIT SEQUENCE STARTS//////////////////
    ///////////////////////////////////////////////////////////

    //Setup for RHS2116 Registers
    RHXRegisters *chipRegisters = new RHXRegisters(rhxController->getType(), rhxController->getSampleRate());
    double dspCutoffFreq;
    dspCutoffFreq = chipRegisters->setDspCutoffFreq(10.0);  // 10 Hz DSP cutoff
    cout << " Actual DSP cutoff frequency: " << dspCutoffFreq << " Hz" << endl;
    chipRegisters->setLowerBandwidth(1.0);      // 1.0 Hz lower bandwidth
    chipRegisters->setUpperBandwidth(7500.0);   // 7.5 kHz upper bandwidth

    
    // First, let's create a command list for the AuxCmd1 slot to configure and read back the RHS chip registers.
    int commandSequenceLength;
    vector<unsigned int> commandList;
    commandSequenceLength = chipRegisters->createCommandListRHSRegisterConfig(commandList, true);

    // Upload command sequence to AuxCmd1.
    rhxController->uploadCommandList(commandList, RHXController::AuxCmd1);
    rhxController->selectAuxCommandLength(RHXController::AuxCmd1, 0, commandSequenceLength - 1);
    rhxController->printCommandList(commandList); //print the command list


    //creating a command to create sin wave of 100Hz and Amplitude of 128 (in DAC steps??, range 0-128)
    commandSequenceLength = chipRegisters->createCommandListZcheckDac(commandList, 100.0, 128.0);
    rhxController->uploadCommandList(commandList, RHXController::AuxCmd2);
    rhxController->selectAuxCommandLength(RHXController::AuxCmd2, 0, commandSequenceLength - 1);
    rhxController->printCommandList(commandList); //print the command list


    // We'll upload dummy command sequences to slots AuxCmd3 and AuxCmd4.
    commandSequenceLength = chipRegisters->createCommandListDummy(commandList, 128, chipRegisters->createRHXCommand(RHXRegisters::RHXCommandRegRead, 255));
    rhxController->uploadCommandList(commandList, RHXController::AuxCmd3);
    rhxController->uploadCommandList(commandList, RHXController::AuxCmd4);
    rhxController->selectAuxCommandLength(RHXController::AuxCmd3, 0, commandSequenceLength - 1);
    rhxController->selectAuxCommandLength(RHXController::AuxCmd4, 0, commandSequenceLength - 1);
    rhxController->printCommandList(commandList); //print the command list

    // Since our longest command sequence is 128 commands, let's just run the SPI interface for 128 samples.
    rhxController->setMaxTimeStep(128);
    rhxController->setContinuousRunMode(false);

    // Start SPI interface.
    rhxController->run();
    // Wait for the 128-sample run to complete.
    while (rhxController->isRunning()) { }

    //[VERIFICATION]: We should observe a sin wave with 100Hz Frequency and 128 DAC (??) Amplitude ?

    ///////////////////////////////////////////////////////////
    ////////////REGISTER INIT SEQUENCE ENDS///////////////////
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
