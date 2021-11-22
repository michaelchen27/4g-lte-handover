#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include <ns3/config-store.h>
#include <ns3/netanim-module.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/ipv4-static-routing-helper.h>

using namespace ns3;

int main (int argc, char *argv[]) {

	// LTE Helper Object
	// Instantiate common objects and provide methods to add eNBs and UEs and configure them.	
	Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

	// Create Evolved Packet Core (EPC), use the EPC Helper class to take care of creating the EPC entities and network.
	Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();


	//Setup EPC to the LTE
	lteHelper->SetEpcHelper(epcHelper); //TODO: ERROR!


	//Configure Scheduler and Handover algo
	lteHelper->SetSchedulerType ("ns3::PssFfMacScheduler");    // PSS scheduler
	lteHelper->SetHandoverAlgorithmType ("ns3::A2A4RsrqHandoverAlgorithm");
	lteHelper->SetHandoverAlgorithmAttribute ("ServingCellThreshold", UintegerValue (30));
	lteHelper->SetHandoverAlgorithmAttribute ("NeighbourCellOffset", UintegerValue (1));


	//Setup PGW
	Ptr<Node> pgw = epcHelper->GetPgwNode ();

	 // Create a single RemoteHost
	NodeContainer remoteHostContainer;
	remoteHostContainer.Create (1);
	Ptr<Node> remoteHost = remoteHostContainer.Get (0);
	InternetStackHelper internet;
	internet.Install (remoteHostContainer);	
 
	//Static Routing Helper
	Ipv4StaticRoutingHelper ipv4RoutingHelper;	

	// Empty nodes without LTE Protocol stack
	// Create Node objects for eNBs, create 3 eNB Nodes.
	NodeContainer enbNodes;
	enbNodes.Create (3);
	

	// Create Node objects for UEs, create 4 user nodes.
	NodeContainer ueNodes;
	ueNodes.Create (4);
	internet.Install (ueNodes);

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



	
	//Setup IPv4 to all UEs, assign IP Address to UEs
	for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
		Ptr<Node> ue = ueNodes.Get(u);
		Ptr<NetDevice> ueLteDevice = ueDevs.Get(u);
		Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer (ueLteDevice));

		//Set default Gateway for the UE
		Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4>());
		ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
	}


	// Activate data radio bearer between each UE and eNB
	enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
	EpsBearer bearer (q);
	lteHelper-> ActivateDataRadioBearer(ueDevs, bearer);


	// Set the stop time, otherwise it will run forever.
	Simulator::Stop (Seconds(0.005));


	//Setup NetAnim
	AnimationInterface anim("handover.xml");
	anim.SetConstantPosition(pgw, 91.0, 35.0);	

	anim.SetConstantPosition(enbNodes.Get(0), 20.0, 20.0);
	anim.SetConstantPosition(enbNodes.Get(1), 40.0, 55.0);
	anim.SetConstantPosition(enbNodes.Get(2), 20.0, 80.0);
	
	anim.SetConstantPosition(enbNodes.Get(0), 25.0, 15.0);
	anim.SetConstantPosition(enbNodes.Get(1), 30.0, 50.0);
	anim.SetConstantPosition(enbNodes.Get(2), 15.0, 75.0);

	anim.SetConstantPosition(remoteHostContainer.Get(0), 91.0, 75.0);



	Simulator::Run();

	//Cleanup and Exit
	Simulator::Destroy();
	return 0;



	











}
