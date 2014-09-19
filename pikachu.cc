#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/constant-position-mobility-model.h"
#include <string>

using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE ("pikachu");

/*void setPos (Ptr<Node> n, int x, int y, int z)
{
  Ptr<ConstantPositionMobilityModel> loc = CreateObject<ConstantPositionMobilityModel> ();
  n->AggregateObject (loc);
  Vector locVec2 (x, y, z);
  loc->SetPosition (locVec2);
}*/

string RemoveComma (std::string& str) 
{
int i = 0;
std::cout<<"remove comma from "<< str << std::endl;
std::string str2=str;
for (i=0; i<2; i++)
{
	std::size_t found=str.find(',');
	if (found!=std::string::npos)
	{
	str2 = str.replace(str.find(','),1," ");
	} else {
	std::cout<<"no comma found.."<<std::endl;
	}
}
std::cout<<"result:"<<str2<<endl;
return str2;
}

string SplitLastValue (const std::string& str)
{
  std::cout << "Splitting: " << str << '\n';
  unsigned found = str.find_last_of(" ");
  ostringstream temp;
  temp << str.substr(found+1);
  return temp.str();
}

int main (int argc, char *argv[])
{
  uint32_t nRtrs = 2;
  
  string tcp_mem = "17949 23932 35898"; //default nexus 5 tcp_mem
  string tcp_rmem = "524288 1048576 2097152"; //default nexus 5 tcp_rmem
  string tcp_rmem_max = "2097152";
  string tcp_wmem = "262144 524288 1048576"; //default nexus 5 tcp_wmem
  string tcp_wmem_max = "1048576";
  string tcp_config_server = "4096 8192 8388608";
  string tcp_config_server_max = "8388608";
  string tcp_config="";
  
  float interval = 0.1;
  
  CommandLine cmd;
  cmd.AddValue ("tcp_mem", "tcp_mem value, put 3 values (bytes) separated by comma, no space after comma. ex: 17949,23932,35898", tcp_mem);
  cmd.AddValue ("tcp_config", "tcp read and write memory in bytes, separated by comma, no space after comma. ex: 524288,1048576,2097152", tcp_config);
  cmd.AddValue ("tcp_config_server", "tcp read and write memory in bytes, separated by comma, no space after comma. ex: 4096,8192,8388608", tcp_config_server);
  cmd.AddValue ("interval", "interval time to connect and disconnect from wifi 1 to wifi 2", interval);
  cmd.Parse (argc, argv);
	
	tcp_mem=RemoveComma(tcp_mem);
	tcp_config_server=RemoveComma(tcp_config_server);
	
  if (tcp_config.size() > 0){
  //remove the comma first
	tcp_config=RemoveComma(tcp_config);
	tcp_rmem = tcp_config;
	tcp_wmem = tcp_config;
  }else{
  cout << "you don't enter tcp_config values, returning to default" <<endl;
  }
	
	tcp_rmem_max= SplitLastValue(tcp_rmem);
	tcp_wmem_max= SplitLastValue(tcp_wmem);
	tcp_config_server_max= SplitLastValue(tcp_config_server);
  
  cout<<"done splitting, values are "<< tcp_rmem_max <<", "<< tcp_wmem_max <<", "<< tcp_config_server_max<<endl;
  
  NodeContainer node, router;
  node.Create (2);
  router.Create (3);

  DceManagerHelper dceManager;
  dceManager.SetTaskManagerAttribute ("FiberManagerType",
                                      StringValue ("UcontextFiberManager"));
	cout<<"fibermanager: OK"<< endl;
  dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory",
                              "Library", StringValue ("liblinux.so"));
	cout<<"liblinux: OK"<<endl;
  LinuxStackHelper stack;
  stack.Install (node);
  stack.Install (router);
  
  dceManager.Install (node);
  dceManager.Install (router);
  cout<<"dce installation: OK"<< endl;
  cout<<"setting up tcp_mem..";
  
  stack.SysctlSet (node.Get(0), ".net.ipv4.tcp_wmem", tcp_wmem);
  stack.SysctlSet (node.Get(0), ".net.ipv4.tcp_rmem", tcp_rmem);
  stack.SysctlSet (node.Get(0), ".net.ipv4.tcp_mem", tcp_mem);
  stack.SysctlSet (node.Get(1), ".net.ipv4.tcp_wmem", tcp_config_server);
  stack.SysctlSet (node.Get(1), ".net.ipv4.tcp_rmem", tcp_config_server);
  stack.SysctlSet (node.Get(1), ".net.ipv4.tcp_mem", tcp_config_server);
                   
  stack.SysctlSet (node.Get(0), ".net.core.rmem_max", tcp_rmem_max);
  stack.SysctlSet (node.Get(0), ".net.core.wmem_max", tcp_wmem_max);
                   
  stack.SysctlSet (node.Get(2), ".net.core.rmem_max", tcp_config_server_max);
  stack.SysctlSet (node.Get(2), ".net.core.wmem_max", tcp_config_server_max);
                   
  stack.SysctlSet (node, ".net.ipv4.tcp_congestion_control", "reno");
	
  cout<<"done"<<endl;
  cout<<"building topology"<<endl;
  
  PointToPointHelper p2p;
  cout<<"p2p"<<endl;
  NetDeviceContainer devices1, devices2, devices3;
  cout<<"devices"<<endl;
  Ipv4AddressHelper address1, address2;
  cout<<"ip4addr"<<endl;
  
  std::ostringstream cmd_oss;
  address1.SetBase ("10.1.0.0", "255.255.255.0");
  address2.SetBase ("10.2.0.0", "255.255.255.0");
  cout<<"ip4addr base"<<endl;
  
  Ptr<RateErrorModel> em = CreateObjectWithAttributes<RateErrorModel> ("RanVar", StringValue ("ns3::UniformRandomVariable[Min=0.0,Max=2.0]"), "ErrorRate", DoubleValue (0.01),"ErrorUnit", EnumValue (RateErrorModel::ERROR_UNIT_PACKET));
  
  Ptr<RateErrorModel> ew = CreateObjectWithAttributes<RateErrorModel> ("RanVar", StringValue ("ns3::UniformRandomVariable[Min=0.0,Max=1.0]"), "ErrorRate", DoubleValue (0.01),"ErrorUnit", EnumValue (RateErrorModel::ERROR_UNIT_PACKET));
  cout<<"error model"<<endl;
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  LinuxStackHelper::PopulateRoutingTables ();
	cout<<"populate routing tables"<<endl;
	
	cout<<"p2p building node 0"<<endl;
  //left side, these are configurations for node 0
	/*this one is for client to p2p LTE*/
	p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("15ms"));
    devices1 = p2p.Install (node.Get (0), router.Get (0));
	devices1.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
	// Assign ip addresses
    Ipv4InterfaceContainer ltelink = address1.Assign (devices1);
    address1.NewNetwork ();
	
	/*client to p2p wifi1*/
	p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("15ms"));
    devices2 = p2p.Install (node.Get (0), router.Get (1));
	devices2.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (ew));
	// Assign ip addresses
    Ipv4InterfaceContainer wifi1 = address1.Assign (devices2);
    address1.NewNetwork ();
	
	/*client to p2p wifi2*/
	p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("15ms"));
    devices3 = p2p.Install (node.Get (0), router.Get (2));
	devices3.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (ew));
	// Assign ip addresses
    Ipv4InterfaceContainer wifi2 = address1.Assign (devices3);
    address1.NewNetwork ();
	
	cmd_oss.str ("");
    cmd_oss << "rule add from " << ltelink.GetAddress (0, 0) << " table 1";
    LinuxStackHelper::RunIp (node.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
	cmd_oss.str ("");
	cmd_oss << "rule add from " << wifi1.GetAddress (0, 0) << " table 2";
    LinuxStackHelper::RunIp (node.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
	cmd_oss.str ("");
	cmd_oss << "rule add from " << wifi2.GetAddress (0, 0) << " table 3";
    LinuxStackHelper::RunIp (node.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
  
	cmd_oss.str ("");
    cmd_oss << "route add 10.1.0.0/24 dev sim0 scope link table 1";
    LinuxStackHelper::RunIp (node.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
	cmd_oss.str ("");
    cmd_oss << "route add 10.1.1.0/24 dev sim1 scope link table 2";
    LinuxStackHelper::RunIp (node.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
	cmd_oss.str ("");
    cmd_oss << "route add 10.1.2.0/24 dev sim2 scope link table 3";
    LinuxStackHelper::RunIp (node.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
	
	cmd_oss.str ("");
    cmd_oss << "route add default via " << ltelink.GetAddress (1, 0) << " dev sim0 table 1";
    LinuxStackHelper::RunIp (node.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
	cmd_oss.str ("");
    cmd_oss << "route add default via " << wifi1.GetAddress (1, 0) << " dev sim1 table 2";
    LinuxStackHelper::RunIp (node.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
	cmd_oss.str ("");
    cmd_oss << "route add default via " << wifi2.GetAddress (1, 0) << " dev sim2 table 3";
    LinuxStackHelper::RunIp (node.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
    
	cout<<"p2p building node 1"<<endl;
  //right side, configuration for node 1
	/*this one is for client to p2p LTE*/
	p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("15ms"));
    devices1 = p2p.Install (node.Get (1), router.Get (0));
	// Assign ip addresses
    ltelink = address2.Assign (devices1);
    address2.NewNetwork ();
	
	/*client to p2p wifi1*/
	p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("15ms"));
    devices2 = p2p.Install (node.Get (1), router.Get (1));
	// Assign ip addresses
    wifi1 = address2.Assign (devices2);
    address2.NewNetwork ();
	
	/*client to p2p wifi2*/
	p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("15ms"));
    devices3 = p2p.Install (node.Get (1), router.Get (2));
	// Assign ip addresses
    wifi2 = address2.Assign (devices3);
    address2.NewNetwork ();
	
	cmd_oss.str ("");
    cmd_oss << "rule add from " << ltelink.GetAddress (0, 0) << " table 1";
	
	
    LinuxStackHelper::RunIp (node.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());
	cmd_oss.str ("");
	cmd_oss << "rule add from " << wifi1.GetAddress (0, 0) << " table 2";
    LinuxStackHelper::RunIp (node.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());
	cmd_oss.str ("");
	cmd_oss << "rule add from " << wifi2.GetAddress (0, 0) << " table 3";
    LinuxStackHelper::RunIp (node.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());
  
	cmd_oss.str ("");
    cmd_oss << "route add 10.2.0.0/24 dev sim0 scope link table 1";
    LinuxStackHelper::RunIp (node.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());
	cmd_oss.str ("");
    cmd_oss << "route add 10.2.1.0/24 dev sim1 scope link table 2";
    LinuxStackHelper::RunIp (node.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());
	cmd_oss.str ("");
    cmd_oss << "route add 10.2.2.0/24 dev sim2 scope link table 3";
    LinuxStackHelper::RunIp (node.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());
	
	cmd_oss.str ("");
    cmd_oss << "route add default via " << ltelink.GetAddress (1, 0) << " dev sim0 table 1";
    LinuxStackHelper::RunIp (node.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());
	cmd_oss.str ("");
    cmd_oss << "route add default via " << wifi1.GetAddress (1, 0) << " dev sim1 table 2";
    LinuxStackHelper::RunIp (node.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());
	cmd_oss.str ("");
    cmd_oss << "route add default via " << wifi2.GetAddress (1, 0) << " dev sim2 table 3";
    LinuxStackHelper::RunIp (node.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());
	/*cout<<"router position"<<endl;
    setPos (router.Get (0), 50, 30, 0);
	setPos (router.Get (1), 50, 20, 0);
	setPos (router.Get (2), 50, 10, 0);
	*/
	cmd_oss.str ("");
    cmd_oss << "route add 10.1.0.0/16 via " << ltelink.GetAddress (1, 0) << " dev sim0";
    LinuxStackHelper::RunIp (router.Get (0), Seconds (0.2), cmd_oss.str ().c_str ());
	cmd_oss.str ("");
    cmd_oss << "route add 10.1.0.0/16 via " << ltelink.GetAddress (1, 0) << " dev sim0";
    LinuxStackHelper::RunIp (router.Get (1), Seconds (0.2), cmd_oss.str ().c_str ());
	cmd_oss.str ("");
    cmd_oss << "route add 10.1.0.0/16 via " << ltelink.GetAddress (1, 0) << " dev sim0";
    LinuxStackHelper::RunIp (router.Get (2), Seconds (0.2), cmd_oss.str ().c_str ());
	
	cmd_oss.str ("");
    cmd_oss << "route add 10.2.0.0/16 via " << ltelink.GetAddress (1, 0) << " dev sim1";
    LinuxStackHelper::RunIp (router.Get (0), Seconds (0.2), cmd_oss.str ().c_str ());
	cmd_oss.str ("");
    cmd_oss << "route add 10.2.0.0/16 via " << wifi1.GetAddress (1, 0) << " dev sim1";
    LinuxStackHelper::RunIp (router.Get (1), Seconds (0.2), cmd_oss.str ().c_str ());
	cmd_oss.str ("");
    cmd_oss << "route add 10.2.0.0/16 via " << wifi1.GetAddress (1, 0) << " dev sim1";
    LinuxStackHelper::RunIp (router.Get (2), Seconds (0.2), cmd_oss.str ().c_str ());

  // default route
  LinuxStackHelper::RunIp (node.Get (0), Seconds (0.1), "route add default via 10.1.0.2 dev sim0");
  LinuxStackHelper::RunIp (node.Get (1), Seconds (0.1), "route add default via 10.2.0.2 dev sim0");
  //LinuxStackHelper::RunIp (node.Get (0), Seconds (0.1), "rule show");
  cout<<"default route"<<endl;

  // debug
  stack.SysctlSet (node, ".net.mptcp.mptcp_debug", "1");
  
  
  cout<<"iperf"<<endl;
  DceApplicationHelper dce;
  ApplicationContainer app1, app2;

  dce.SetStackSize (1 << 20);

  // Launch iperf client on node 1
  dce.SetBinary ("iperf");
  dce.ResetArguments ();
  dce.ResetEnvironment ();
  dce.AddArgument ("-c");
  dce.AddArgument ("10.1.0.1");
  dce.AddArgument ("-i");
  dce.AddArgument ("3");
  dce.AddArgument ("--time");
  dce.AddArgument ("10");

  app1 = dce.Install (node.Get (1));
  app1.Start (Seconds (1.1));
  app1.Stop (Seconds (20));

  // Launch iperf server on node 0
  dce.SetBinary ("iperf");
  dce.ResetArguments ();
  dce.ResetEnvironment ();
  dce.AddArgument ("-s");
  app2 = dce.Install (node.Get (0));

  p2p.EnablePcapAll ("pikachu-mptcp");  
  app2.Start (Seconds (1));
  
  cout<<"up down interface"<<endl;
  //connect-disconnect
  LinuxStackHelper::RunIp (router.Get (2), Seconds (0.11), "link set sim0 up");
  LinuxStackHelper::RunIp (router.Get (2), Seconds (0.15), "link set sim0 down");
  LinuxStackHelper::RunIp (router.Get (2), Seconds (5+interval), "link set sim0 up");
  LinuxStackHelper::RunIp (router.Get (1), Seconds (0.11), "link set sim0 up");
  LinuxStackHelper::RunIp (router.Get (1), Seconds (5), "link set sim0 down");
  /*cmd_oss.str ("");
  cmd_oss << "link set down dev sim1";
  LinuxStackHelper::RunIp (node.Get (0), Seconds (5), cmd_oss.str ().c_str ());
  cmd_oss.str ("");
  cmd_oss << "link set down dev sim2";
  LinuxStackHelper::RunIp (node.Get (0), Seconds (0.5), cmd_oss.str ().c_str ());
  cmd_oss.str ("");
  cmd_oss << "link set up dev sim2";
  LinuxStackHelper::RunIp (node.Get (0), Seconds (5+interval), cmd_oss.str ().c_str ());
  */
  /*
  setPos (node.Get (0), 0, 20, 0);
  setPos (node.Get (1), 100, 20, 0);
  */
  Simulator::Stop (Seconds (30));
  cout<<"start simulation"<<endl;
  Simulator::Run ();
  Simulator::Destroy ();
  cout<<"end simulation"<<endl;

  return 0;
}

