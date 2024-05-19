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
    rhxController->setCableLengthFeet(PortA, 3.0);

    // Let's turn one LED on to indicate that the program is running.
    int ledArray[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    rhxController->setLedDisplay(ledArray);

    //[VERIFIED: THE CODE TILL ABOVE HAS BEEN VERIFIED]

    ///////////////////////////////////////////////////////////
    /////////////////////INIT SEQUENCE ENDS////////////////////
    ///////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////
    ////////////REGISTER INIT SEQUENCE STARTS//////////////////
    ///////////////////////////////////////////////////////////

    //safety measure to make sure this is always run with real hardware
    if (rhxController->isSynthetic() || rhxController->isPlayback()){
        std::cout << "A REAL XEM7310 IS NOT CONNECTED, DISCONNECTING" ;
        return 0;
    } 

    RHXRegisters *chipRegisters = new RHXRegisters(rhxController->getType(), rhxController->getSampleRate(), StimStepSize10nA);



    const int Never = 65535; // a const defined by intan

    int stream = 0; // [TO SET]: I think this is about the total RHS2116, so (1-8) I think
    int channel = 0; // [TO SET]: I think this is talking about the electrodes so there should be 32 per chip
    int triggerSource = 25; // [TO SET] 0-15 is Digital in Port on FPGA, 16-23 is Analog I/O on FPGA, 24-31 software triggers (F1-F8)

    //for now 16 should be ok, later replace this with: chipRegisters->maxNumChannelsPerChip(ControllerStimRecord);
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
    int eventAmpSettleOn = Never; //got this from Intan RHX github line 1299
    int eventAmpSettleOff = 0; //got this from Intan RHX github line 1300
    int eventAmpSettleOnRepeat = Never; //got this from Intan RHX github line 1301
    int eventAmpSettleOffRepeat = Never; //got this from Intan RHX github line 1302
    int eventChargeRecovOn = Never; //got this from Intan RHX github line 1308
    int eventChargeRecovOff = 0; //got this from Intan RHX github line 1309
    int dacBaseline = 32768; //32768 is the midvalue, don't worry about this now, since it is concerned with after stim effects and affects reading
    int dacPositive = 32768; //same
    int dacNegative = 32768; //same


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

    chipRegisters->setStimEnable(true); // this enables Stimulation 
    chipRegisters->setStimStepSize(StimStepSize10nA); //setting the stim stepSize 10nA

    for (int i = 0; i < totalChannels; i++){
        //first let's just test this for the 16 electrodes
        int magSetter1 = chipRegisters->setPosStimMagnitude(i, 200, 0); //[TO CHECK]: check these values just to be sure, but they should mostly work
        int magSetter2 = chipRegisters->setNegStimMagnitude(i, 200, 0); //[TO CHECK]: check these values just to be sure, but they should mostly work

        if (magSetter1 == -1 || magSetter2 == -1){
            std::cout << "ERROR WHILE SETTING MAGNITUDE: OUT OF RANGE";
        }
    }
    
    
    int commandSequenceLength;
    vector<unsigned int> commandList;
    commandSequenceLength = chipRegisters->createCommandListSetStimMagnitudes(commandList, channel, 200, 0, 200, 0);
    rhxController->uploadCommandList(commandList, AbstractRHXController::AuxCmd1, 0);
    rhxController->selectAuxCommandLength(AbstractRHXController::AuxCmd1, 0, commandSequenceLength - 1);
    chipRegisters->createCommandListDummy(commandList, 8192, chipRegisters->createRHXCommand(RHXRegisters::RHXCommandRegRead, 255));
    rhxController->uploadCommandList(commandList, AbstractRHXController::AuxCmd2, 0);
    rhxController->uploadCommandList(commandList, AbstractRHXController::AuxCmd3, 0);
    rhxController->uploadCommandList(commandList, AbstractRHXController::AuxCmd4, 0);

    rhxController->setMaxTimeStep(commandSequenceLength);
    rhxController->setContinuousRunMode(false);
    rhxController->setStimCmdMode(false); //we want the autostimcommand mode off, cause then the stim commands are ignored and the aux1,2,3,4 commands are talen
    //rhxController->enableAuxCommandsOnOneStream(stream);

    rhxController->setMaxTimeStep(commandSequenceLength);
    rhxController->setStimCmdMode(false);
    //rhxController->enableAuxCommandsOnOneStream(stream);
    rhxController->enableAuxCommandsOnAllStreams();


    rhxController->setContinuousRunMode(true);
    // Start SPI interface.
    rhxController->run();
    // Wait for the 128-sample run to complete.
    while (rhxController->isRunning()) { }

    //the next part is for safety
    commandSequenceLength = chipRegisters->createCommandListRHSRegisterRead(commandList);
    rhxController->uploadCommandList(commandList, AbstractRHXController::AuxCmd1, 0);
    rhxController->selectAuxCommandLength(AbstractRHXController::AuxCmd1, 0, commandSequenceLength - 1);
    rhxController->run();
    while (rhxController->isRunning() ) {}

    rhxController->enableAuxCommandsOnAllStreams();

    //[VERIFICATION]: When we press F2, should observe a square wave with the above parameters

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