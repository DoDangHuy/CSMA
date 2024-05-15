#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiAdhoc");

int main(int argc, char *argv[])
{
    bool verbose = true;
    uint32_t numNodes = 30;

    CommandLine cmd;
    cmd.AddValue("numNodes", "Number of nodes", numNodes);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    cmd.Parse(argc, argv);

    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }
    // Create nodes
    NodeContainer nodes;
    nodes.Create(numNodes);


    // Create wifi channels
    // To set up the parameters for the transmission channel and physical attributes such as transmission power, receiver sensitivity, and radio wave propagation models, 
    // YansWifiChannelHelper and YansWifiPhyHelper are used to create and configure the channel.
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetChannel(wifiChannel.Create());

    // Set wifi mac parameters
    // MAC Layer - In case of spontaneous mode, the MAC layer uses the CSMA/CA protocol to avoid collisions by listening to the wireless medium before transmitting data.
    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");


    // Set wifi network interface
    // responsible for data transmission between devices in close proximity without having to go through any router

    WifiHelper wifi;
    //Disable RTS/CTS by setting the threshold to 0
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "RtsCtsThreshold", UintegerValue(0));
    NetDeviceContainer wifiDevices;
    wifiDevices = wifi.Install(wifiPhy, wifiMac, nodes);

    // Set mobility model
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(10.0),
                                  "DeltaY", DoubleValue(10.0),
                                  "GridWidth", UintegerValue(3),
                                  "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // Install internet stack on nodes
    InternetStackHelper internet;
    internet.Install(nodes);

    //Ipv4AddressHelper assigns IP addresses to interfaces on network nodes. 
    //it automatically assigns IP addresses to each network interface in the NetDeviceContainer that you pass in
    //10.1.1.0 is the IP address used as the starting point.
    //10.1.1.255 is the broadcast address
    //255.255.255.0 is the subnet mask. The first 24 bits of the IP address are the network part and the last 8 bits are the part for the network nodes in that subnet.
    Ipv4AddressHelper wifiAddress;
    wifiAddress.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer wifiInterfaces;
    wifiInterfaces = wifiAddress.Assign(wifiDevices);

    // Set up the server application on the first node
    // UdpEchoClientHelper will create a UDP socket by default when you install it on a node.
    UdpEchoServerHelper server(9);
    ApplicationContainer serverApp = server.Install(nodes.Get(0));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));
    
    //Create an UDPEchoClientHelper
    UdpEchoClientHelper client(wifiInterfaces.GetAddress(0, 0), 9);
    client.SetAttribute("MaxPackets", UintegerValue(10));
    client.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    client.SetAttribute("PacketSize", UintegerValue(1024));
    
    // Install the client on the rest of the nodes
    for (uint32_t i = 1; i < numNodes; i++)
    {
        ApplicationContainer clientApp = client.Install(nodes.Get(i));0
        clientApp.Start(Seconds(2.0));
        clientApp.Stop(Seconds(10.0));
    }

    // Configure flow monitor
    FlowMonitorHelper flowMon;
    Ptr<FlowMonitor> monitor = flowMon.InstallAll();

    // Run simulation
    Simulator::Stop(Seconds(15.0));
    Simulator::Run();

    //Save collected data into xml file
    monitor->SerializeToXmlFile("Data.xml", true, true);

    Simulator::Destroy();
    
    return 0;
}




