#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/lte-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store-module.h"
#include "ns3/netanim-module.h"
#include "ns3/gnuplot.h"
#include <math.h>

using namespace ns3;

int main (int argc, char *argv[]) {

	// LTE Helper Object
	// Instantiate common objects and provide methods to add eNBs and UEs and configure them.	
	Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

	// Create Evolved Packet Core (EPC), use the EPC Helper class to take care of creating the EPC entities and network.
	Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();

	//Configure Scheduler and Handover algo
	lteHelper->SetSchedulerType ("ns3::FdMtFfMacScheduler");    // FD-MT scheduler
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

	// Create the internet
	PointToPointHelper p2ph;
	p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
	p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
	p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
	NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
	Ipv4AddressHelper ipv4h;
	ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
	Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
	// interface 0 is localhost, 1 is the p2p device
	//Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

	// Routing of the Internet Host (towards the LTE network)
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  // interface 0 is localhost, 1 is the p2p device
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);



	// Empty nodes without LTE Protocol stack
	// Create Node objects for eNBs, create 3 eNB Nodes.
	NodeContainer enbNodes;
	enbNodes.Create (3);
	

	// Create Node objects for UEs, create 4 user nodes.
	NodeContainer ueNodes;
	ueNodes.Create (4);

	// Mobility model for all nodes
	MobilityHelper mobility;
	mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (10.0),
                                 "MinY", DoubleValue (10.0),
                                 "DeltaX", DoubleValue (12.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (5),
                                 "LayoutType", StringValue ("RowFirst"));

	mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue (Rectangle (-500, 500, -250, 500)));
  mobility.Install (ueNodes);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (enbNodes);
/*
mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
mobility.Install (enbNodes);
mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");	
mobility.Install (ueNodes);
*/

	// Install LTE protocol stack on eNBs
	NetDeviceContainer enbDevs;
	enbDevs = lteHelper->InstallEnbDevice (enbNodes);


	// Install LTE protocol stack on UEs
	NetDeviceContainer ueDevs;
	ueDevs = lteHelper->InstallUeDevice (ueNodes);


	// TODO: Attach UEs to eNB, configure each UE according to the eNB configs, create RRC Connection between them.
	lteHelper->Attach (ueDevs, enbDevs.Get(0));
	internet.Install(ueNodes);
	
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

	//Setup EPC to the LTE
	lteHelper->SetEpcHelper(epcHelper);



	// Set the stop time, otherwise it will run forever.
	Simulator::Stop (Seconds(30.0));


	//Setup NetAnim
	AnimationInterface anim("handover.xml");
	anim.SetMaxPktsPerTraceFile(9999999999);

	// Set Labels to Nodes	
	anim.UpdateNodeDescription(remoteHost, "Remote Host Container");
	anim.UpdateNodeDescription(pgw, "Packet Data Network Gateway (PGW)");
	anim.UpdateNodeDescription(enbNodes.Get(0), "eNB 0");
	anim.UpdateNodeDescription(enbNodes.Get(1), "eNB 1");
	anim.UpdateNodeDescription(enbNodes.Get(2), "eNB 2");
	anim.UpdateNodeDescription(ueNodes.Get(0), "UE 0");
	anim.UpdateNodeDescription(ueNodes.Get(1), "UE 1");
	anim.UpdateNodeDescription(ueNodes.Get(2), "UE 2");
	anim.UpdateNodeDescription(ueNodes.Get(3), "UE 3");

	// Set constant positions
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
