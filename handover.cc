#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include <ns3/config-store.h>
#include <ns3/netanim-module.h>

using namespace ns3;

int main (int argc, char *argv[]) {

	// LTE Helper Object
	// Instantiate common objects and provide methods to add eNBs and UEs and configure them.	
	Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
	
	//Configure Scheduler and Handover algo
	lteHelper->SetSchedulerType ("ns3::PssFfMacScheduler");    // PSS scheduler
	lteHelper->SetHandoverAlgorithmType ("ns3::A2A4RsrqHandoverAlgorithm");
	lteHelper->SetHandoverAlgorithmAttribute ("ServingCellThreshold", UintegerValue (30));
	lteHelper->SetHandoverAlgorithmAttribute ("NeighbourCellOffset", UintegerValue (1));


	// Create Evolved Packet Core (EPC), use the EPC Helper class to take care of creating the EPC entities and network.
	Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();


	// Tell LTE helper to use the EPC
	//lteHelper->SetEpcHelper(epcHelper); //TODO: This code still produces ERROR
	

	//Setup PGW
	Ptr<Node> pgw = epcHelper->GetPgwNode ();
 

	// Empty nodes without LTE Protocol stack
	// Create Node objects for eNBs, create 3 eNB Nodes.
	NodeContainer enbNodes;
	enbNodes.Create (3);
	

	// Create Node objects for UEs, create 4 user nodes.
	NodeContainer ueNodes;
	ueNodes.Create (4);


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

	//Setup NetAnim
	AnimationInterface anim("handover.xml");
	anim.SetConstantPosition(pgw, 91.0, 20.0, 20.0);

	Simulator::Run();

	//Cleanup and Exit
	Simulator::Destroy();
	return 0;



	











}
