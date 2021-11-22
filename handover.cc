#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>

using namespace ns3;

int main (int argc, char *argv[]) {

	// LTE Helper Object
	// Instantiate common objects and provide methods to add eNBs and UEs and configure them.
	
	Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

	// Empty nodes without LTE Protocol stack
	
	// Create Node objects for eNBs
	NodeContainer enbNodes;
	enbNodes.Create (1);
	
	// Create Node objects for UEs
	NodeContainer ueNodes;
	ueNodes.Create (2);

	// Mobility model for all nodes
	// Place all nodes at (0,0,0) coordinates.
	MobilityHelper mobility;
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (enbNodes);
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");	
	mobility.Install (ueNodes);

	// Install LTE protocol stack on eNBs
	NetDeviceContainer enbDevs;
	enbDevs = lteHelper->InstallEnbDevice (enbNodes);


	// Install LTE protocol stack on UEs
	NetDeviceContainer ueDevs;
	ueDevs = lteHelper->InstallUeDevice (ueNodes);

	// Attach UEs to eNB, configure each UE according to the eNB configs, create RRC Connection between them.
	lteHelper->Attach (ueDevs, enbDevs.Get (0));
	
	// Activate data radio bearer between each UE and eNB
	enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
	EpsBearer bearer (q);
	lteHelper-> ActivateDataRadioBearer(ueDevs, bearer);

	// Set the stop time, otherwise it will run forever.
	Simulator::Stop (Seconds(0.005));

	Simulator::Run();

	//Cleanup and Exit
	Simulator::Destroy();
	return 0;



	











}
