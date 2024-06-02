#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
using namespace std;
#include "okFrontPanel.h"
#include "rhxcontroller.h"
#include "rhxregisters.h"
#include "rhxdatablock.h"
int main(int argc, char* argv[])
{
    // Create RHX Controller with 20 kHz per-amplifier sampling rate.
    RHXController *rhxController = new RHXController(ControllerStimRecord,
    SampleRate20000Hz);
    // Open the first detected Opal Kelly device, load RhythmStim USB-7310 bitfile.
    vector<string> availableDevices = rhxController->listAvailableDeviceSerials();
    rhxController->open(availableDevices[0]);
    // Load RhythmStim USB-7310 bitfile and initialize
    rhxController->uploadFPGABitfile("ConfigRHSController_7310.bit");
    rhxController->initialize();
    rhxController->enableDataStream(0, true);
    // We can set the MISO sampling delay which is dependent on the sample rate.
    // We assume a 3-foot cable.
    rhxController->setCableLengthFeet(PortA, 3.0);
    // Let's turn one LED on to indicate that the program is running.
    int ledArray[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    rhxController->setLedDisplay(ledArray);
    // Set up an RHX register object.
    RHXRegisters *chipRegisters = new RHXRegisters(rhxController->getType(),
    rhxController->getSampleRate());
    // Create command lists to be uploaded to auxiliary command slots.
    int commandSequenceLength;
    vector<unsigned int> commandList;
    
    // First, let's create a command list for the AuxCmd1 slot to configure
    // and read back the RHS chip registers.
    // Before generating register configuration command sequence, set
    // amplifier bandwidth parameters.
    double dspCutoffFreq;
    dspCutoffFreq = chipRegisters->setDspCutoffFreq(10.0); // 10 Hz DSP cutoff
    cout << "Actual DSP cutoff frequency: " << dspCutoffFreq << " Hz" << endl;
    chipRegisters->setLowerBandwidth(1.0); // 1.0 Hz lower bandwidth
    chipRegisters->setUpperBandwidth(7500.0); // 7.5 kHz upper bandwidth
    commandSequenceLength =
    chipRegisters->createCommandListRHSRegisterConfig(commandList, false);
    // Upload command sequence to AuxCmd1.
    rhxController->uploadCommandList(commandList, RHXController::AuxCmd1);
    rhxController->selectAuxCommandLength(RHXController::AuxCmd1, 0,
    commandSequenceLength - 1);
    // rhxController->printCommandList(commandList); // optionally, print command list

    // Next, we'll create a command list for the AuxCmd2 slot. This command
    // will create a 1 kHz, full-scale sine wave for impedance testing.
    // commandSequenceLength =
    // chipRegisters->createCommandListZcheckDac(commandList, 1000.0, 128.0);
    // rhxController->uploadCommandList(commandList, RHXController::AuxCmd2);
    // rhxController->selectAuxCommandLength(RHXController::AuxCmd2, 0,
    // commandSequenceLength - 1);
    // rhxController->printCommandList(commandList); // optionally, print command list

    // We’ll upload dummy command sequences to slots AuxCmd3 and AuxCmd4 (and AuxCmd2).
    commandSequenceLength =
    chipRegisters->createCommandListDummy(commandList, 128,
    chipRegisters->createRHXCommand(RHXRegisters::RHXCommandRegRead, 255));
    rhxController->uploadCommandList(commandList, RHXController::AuxCmd2);
    rhxController->uploadCommandList(commandList, RHXController::AuxCmd3);
    rhxController->uploadCommandList(commandList, RHXController::AuxCmd4);
    rhxController->selectAuxCommandLength(RHXController::AuxCmd2, 0,
    commandSequenceLength - 1);
    rhxController->selectAuxCommandLength(RHXController::AuxCmd3, 0,
    commandSequenceLength - 1);
    rhxController->selectAuxCommandLength(RHXController::AuxCmd4, 0,
    commandSequenceLength - 1);
    // rhxController->printCommandList(commandList); // optionally, print command list

    // Since our longest command sequence is 128 commands, let’s just run the SPI
    // interface for 128 sample (clock cycles?).
    rhxController->setMaxTimeStep(128);
    rhxController->setContinuousRunMode(false);
    // Start SPI interface.
    rhxController->run();
    // Wait for the 128-sample run to complete.
    while (rhxController->isRunning()) { }
    // Read the resulting single data block from the USB interface.
    RHXDataBlock *dataBlock =
    new RHXDataBlock(rhxController->getType(),
    rhxController->getNumEnabledDataStreams());
    rhxController->readDataBlock(dataBlock);
    // Display register contents from data stream 0.
    dataBlock->print(0);

    // Let's save one second of data to a binary file on disk.
    ofstream saveOut;
    saveOut.open("binary_save_file.dat", ios::binary | ios::out);
    deque<RHXDataBlock*> dataQueue;
    // Run for one second.
    rhxController->setMaxTimeStep(20000);
    rhxController->run();
    bool usbDataRead;
    do {
    usbDataRead = rhxController->readDataBlocks(1, dataQueue);
    if (dataQueue.size() >= 50) { // save 50 data blocks at a time
    rhxController->queueToFile(dataQueue, saveOut);
    }
    } while (usbDataRead || rhxController->isRunning());
    rhxController->queueToFile(dataQueue, saveOut);
    saveOut.close();
    // Turn off LED.
    ledArray[0] = 0;
    rhxController->setLedDisplay(ledArray);
    delete dataBlock;
    delete chipRegisters;
    delete rhxController;
    return 0;
}