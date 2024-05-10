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

    //safety measure to make sure this is always run with real hardware
    if (rhxController->isSynthetic() || rhxController->isPlayback()) return;

    const int Never = 65535; // a const defined by intan

    int stream = 0; // [TO SET]: I think this is about the total RHS2116, so (1-8) I think
    int channel = 0; // [TO SET]: I think this is talking about the electrodes so there should be 32 per chip
    int triggerSource = 25; // [TO SET] 0-15 is Digital in Port on FPGA, 16-23 is Analog I/O on FPGA, 24-31 software triggers (F1-F8)
    int totalChannels = 16;  //we are only trying on the first 16 channels first and then later we can extend this to however many channels we want
    int numPulse = 1; // we can later make this a train instead
    int enabled = 1; //must be one if you want to stim
    StimShape pulseType = Biphasic;
    bool triggerEdge = true; //if we set this false then that means, TriggerLevel instead
    bool triggerLow = true; //i.e on the rising edge 
    bool negStimFirst = true; //Cathodic or Anodic, it is usually Cathodic
    int postTriggerDelay = 0; // we want this to be zero, basically: stim as soon as we press the Trigger
    int firstPhaseDuration = 100; // this is in Microseconds & the value must agree with how fine your StimStepSize is
    //[TO DO] 100 seems like a possible value from the GUI for now, but will have to code up the formula for verification of step size
    int secondPhaseDuration = 100;
    int refractoryPeriod = 100; // let's keep it 100 for now, cause why not

    //individaul register values:

    int eventStartStim = postTriggerDelay;
    int eventStimPhase2 = eventStartStim + firstPhaseDuration;
    int eventStimPhase3 = Never;
    int eventEndStim = eventStimPhase2 + secondPhaseDuration;
    int eventEnd = eventEndStim + refractoryPeriod;
    int eventRepeatStim = Never; // since we are not a PulseTrain and just a single stim
    int eventAmpSettleOn;
    int eventAmpSettleOff;
    int eventAmpSettleOnRepeat;
    int eventAmpSettleOffRepeat;
    int eventChargeRecovOn;
    int eventChargeRecovOff;

    for (int i = 0; i < totalChannels; i++){
        channel = i;
        rhxController->configureStimTrigger(stream, channel, triggerSource, enabled ,triggerEdge, triggerLow); //configures the trigger for the Stim
        rhxController->configureStimPulses(stream, channel, numPulse, pulseType, negStimFirst); //configures the pulses (basic info)
        //now following commands change the values of the RHS2116 registers with the interface:
        // programStimReg(int stream, int channel, StimRegister reg, int value)
        rhxController->programStimReg(stream, channel, AbstractRHXController::TriggerParams, eventStartStim);
        rhxController->programStimReg(stream, channel, AbstractRHXController::EventEndStim, eventEndStim);
        rhxController->programStimReg(stream, channel, AbstractRHXController::EventRepeatStim, eventRepeatStim);
        rhxController->programStimReg(stream, channel, AbstractRHXController::EventEnd, eventEnd);
        rhxController->programStimReg(stream, channel, AbstractRHXController::EventAmpSettleOn, eventAmpSettleOn);
        rhxController->programStimReg(stream, channel, AbstractRHXController::EventStartStim, eventStartStim);
        rhxController->programStimReg(stream, channel, AbstractRHXController::EventStimPhase2, eventStimPhase2);
        rhxController->programStimReg(stream, channel, AbstractRHXController::EventStimPhase3, eventStimPhase3);
        rhxController->programStimReg(stream, channel, AbstractRHXController::EventEndStim, eventEndStim);
        rhxController->programStimReg(stream, channel, AbstractRHXController::EventRepeatStim, eventRepeatStim);
        rhxController->programStimReg(stream, channel, AbstractRHXController::EventAmpSettleOff, eventAmpSettleOff);
        rhxController->programStimReg(stream, channel, AbstractRHXController::EventChargeRecovOn, eventChargeRecovOn);
        rhxController->programStimReg(stream, channel, AbstractRHXController::EventChargeRecovOff, eventChargeRecovOff);
        rhxController->programStimReg(stream, channel, AbstractRHXController::EventAmpSettleOnRepeat, eventAmpSettleOnRepeat);
        rhxController->programStimReg(stream, channel, AbstractRHXController::EventAmpSettleOffRepeat, eventAmpSettleOffRepeat);
        rhxController->programStimReg(stream, channel, AbstractRHXController::EventEnd, eventEnd);
        rhxController->programStimReg(stream, 0, AbstractRHXController::DacBaseline, dacBaseline);
        rhxController->programStimReg(stream, 0, AbstractRHXController::DacPositive, dacPositive);
        rhxController->programStimReg(stream, 0, AbstractRHXController::DacNegative, dacNegative);
    }

    
    

    //Setup for RHS2116 Registers
    

    




    RHXRegisters *chipRegisters = new RHXRegisters(rhxController->getType(), rhxController->getSampleRate(), StimStepSize10nA);
    chipRegisters->setStimEnable(true); // this enables Stimulation, prev we defined the Stim size to be 10nA
    for (int i = 0; i < 16; i++){
        //first let's just test this for the 16 electrodes
        chipRegisters->setPosStimMagnitude(i, 200);
        chipRegisters->setNegStimMagnitude(i, 200);
    }






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
