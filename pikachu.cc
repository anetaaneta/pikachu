#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/constant-position-mobility-model.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <time.h>
#include "rapidxml.hpp"

using namespace ns3;
using namespace std;
//using namespace rapidxml;
NS_LOG_COMPONENT_DEFINE ("pikachu");

void setPos (Ptr<Node> n, int x, int y, int z)
{
  Ptr<ConstantPositionMobilityModel> loc = CreateObject<ConstantPositionMobilityModel> ();
  n->AggregateObject (loc);
  Vector locVec2 (x, y, z);
  loc->SetPosition (locVec2);
}

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

string convertInt(int number)
{
   stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}

string dconvertInt(double number)
{
   stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
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
  int path_num = 3;
  bool xml=true;
  vector<string> bwstring, delaystring, errorstring, udpbwstring;
  int path, count;
  rapidxml::xml_node<> *doku;
  rapidxml::xml_attribute<> *bttr;
  srand (time(NULL));
  
  float interval = 0.1;
  
  CommandLine cmd;
  cmd.AddValue ("tcp_mem", "tcp_mem value, put 3 values (bytes) separated by comma, no space after comma. ex: 17949,23932,35898", tcp_mem);
  cmd.AddValue ("tcp_config", "tcp read and write memory in bytes, separated by comma, no space after comma. ex: 524288,1048576,2097152", tcp_config);
  cmd.AddValue ("tcp_config_server", "tcp read and write memory in bytes, separated by comma, no space after comma. ex: 4096,8192,8388608", tcp_config_server);
  cmd.AddValue ("interval", "interval time to connect and disconnect from wifi 1 to wifi 2", interval);
  cmd.AddValue ("path_num", "number of path to be created between sender and receiver", path_num);
  cmd.AddValue ("xml", "whether xml input is used. default file: note.xml", xml);
  cmd.Parse (argc, argv);
	
	if (xml==true){
		ifstream myfile("note.xml");
		rapidxml::xml_document<> doc;
		vector<char> buffer((istreambuf_iterator<char>(myfile)), istreambuf_iterator<char>( ));
		buffer.push_back('\0');
		
		doc.parse<0>(&buffer[0]);
		rapidxml::xml_node<> *doku = doc.first_node();
		string valstring[5];
		rapidxml::xml_attribute<> *bttr = doku->first_attribute();
		
			path_num = atoi(bttr->value());
			delaystring.resize(path_num);
			bwstring.resize(path_num);
			errorstring.resize(path_num);
			udpbwstring.resize(path_num);
		
		for (rapidxml::xml_node<> *child = doku->first_node("parameters"); child; child=child->next_sibling("parameters")){
			count = 0;
				for (rapidxml::xml_attribute<> *attr = child->first_attribute(); attr; attr = attr->next_attribute())
				{
					valstring[count]=attr->value();
					count++;
				}
			path = atoi (valstring[0].c_str());
			if(path>path_num){
				break;
			}
			delaystring[path-1]=valstring[1];
			bwstring[path-1]=valstring[2];
			errorstring[path-1]=valstring[3];
			udpbwstring[path-1]=valstring[4];
			}
		rapidxml::xml_node<> *tcp = doku->first_node("tcp_mem");
		tcp_mem=tcp->value();
		tcp = doku->first_node("tcp_rmem");
		tcp_rmem=tcp->value();
		tcp = doku->first_node("tcp_wmem");
		tcp_wmem=tcp->value();
		tcp = doku->first_node("tcp_config_server");
		tcp_config_server=tcp->value();
	}else{
		//just put some random number inside. hehehehee
		delaystring.resize(path_num);
		bwstring.resize(path_num);
		for(int b=0; b<path_num; b++){
		delaystring[b]=convertInt(rand() % 10 + 1)+"ms";
		bwstring[b]=convertInt(rand() % 100 + 1)+"Mbps";
		udpbwstring[b]=convertInt (rand()% 20 + 1);
		errorstring[b]=dconvertInt ((rand()% 100 + 1)/10000);
		}
	}
	
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
  
  NodeContainer node, routerSend, routerReceive, nuisanceSend, nuisanceReceive;
  node.Create (2);
  routerSend.Create (path_num);
  routerReceive.Create (path_num);
  nuisanceSend.Create(path_num);
  nuisanceReceive.Create(path_num);

  DceManagerHelper dceManager;
  dceManager.SetTaskManagerAttribute ("FiberManagerType",
                                      StringValue ("UcontextFiberManager"));
  dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory",
                              "Library", StringValue ("liblinux.so"));
  cout<<"liblinux: OK"<<endl;
  LinuxStackHelper stack;
  stack.Install (node);
  stack.Install (routerSend);
  stack.Install (routerReceive);
  stack.Install (nuisanceSend);
  stack.Install (nuisanceReceive);
  
  dceManager.Install (node);
  dceManager.Install (routerSend);
  dceManager.Install (routerReceive);
  dceManager.Install (nuisanceSend);
  dceManager.Install (nuisanceReceive);
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
                   
  stack.SysctlSet (node.Get(1), ".net.core.rmem_max", tcp_config_server_max);
  stack.SysctlSet (node.Get(1), ".net.core.wmem_max", tcp_config_server_max);
  
  stack.SysctlSet (node.Get(0), ".net.core.netdev_max_backlog", "250000");
  stack.SysctlSet (node.Get(1), ".net.core.netdev_max_backlog", "250000");
  stack.SysctlSet (routerSend, ".net.core.netdev_max_backlog", "250000"); 
  stack.SysctlSet (routerReceive, ".net.core.netdev_max_backlog", "250000");
  //just in case..
  stack.SysctlSet (routerSend, ".net.ipv4.tcp_wmem", "8388608 8388608 8388608");
  stack.SysctlSet (routerSend, ".net.ipv4.tcp_rmem", "8388608 8388608 8388608");
  stack.SysctlSet (routerSend, ".net.ipv4.tcp_mem", "8388608 8388608 8388608");
  stack.SysctlSet (routerSend, ".net.core.rmem_max", "8388608");
  stack.SysctlSet (routerSend, ".net.core.wmem_max", "8388608");
  stack.SysctlSet (routerReceive, ".net.ipv4.tcp_wmem", "8388608 8388608 8388608");
  stack.SysctlSet (routerReceive, ".net.ipv4.tcp_rmem", "8388608 8388608 8388608");
  stack.SysctlSet (routerReceive, ".net.ipv4.tcp_mem", "8388608 8388608 8388608");
  stack.SysctlSet (routerReceive, ".net.core.rmem_max", "8388608");
  stack.SysctlSet (routerReceive, ".net.core.wmem_max", "8388608");
  
  stack.SysctlSet (node, ".net.ipv4.tcp_congestion_control", "reno");
	
  
  PointToPointHelper pointToPoint;
  NetDeviceContainer senderlink, receiverlink, snuisancelink, rnuisancelink, r2r;
  Ipv4AddressHelper address1, address2, address3, address4, address5;
  
  std::ostringstream cmd_oss;
  address1.SetBase ("10.1.0.0", "255.255.255.0");
  address2.SetBase ("10.2.0.0", "255.255.255.0");
  address3.SetBase ("10.3.0.0", "255.255.255.0");
  address4.SetBase ("10.4.0.0", "255.255.255.0");
  address5.SetBase ("10.5.0.0", "255.255.255.0");
  cout<<"ip4addr base"<<endl;
  
	for (uint32_t i = 0; i < path_num; i++)
    {
      //create topology of sender to sender's router
      pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("20Gbps"));
      senderlink = pointToPoint.Install (node.Get (0), routerSend.Get (i));
      // Assign ip addresses
      Ipv4InterfaceContainer if1 = address1.Assign (senderlink);
      address1.NewNetwork ();
      // setup ip routes
      cmd_oss.str ("");
      cmd_oss << "rule add from " << if1.GetAddress (0, 0) << " table " << (i+1);
      LinuxStackHelper::RunIp (node.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add 10.1." << i << ".0/24 dev sim" << i << " scope link table " << (i+1);
      LinuxStackHelper::RunIp (node.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add default via " << if1.GetAddress (1, 0) << " dev sim" << i << " table " << (i+1);
      LinuxStackHelper::RunIp (node.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add 10.1."<<i<<".0/24 via " << if1.GetAddress (1, 0) << " dev sim0";
      LinuxStackHelper::RunIp (routerSend.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());
      
      Ptr<RateErrorModel> em = CreateObjectWithAttributes<RateErrorModel> (
      "RanVar", StringValue ("ns3::UniformRandomVariable[Min=0.0,Max=1.0]"),
      "ErrorRate", DoubleValue (atof(errorstring[i].c_str())),
      "ErrorUnit", EnumValue (RateErrorModel::ERROR_UNIT_PACKET)
      );

      
      //link between routers
      pointToPoint.SetDeviceAttribute ("DataRate", StringValue (bwstring[i]));
      pointToPoint.SetChannelAttribute ("Delay", StringValue (delaystring[i]));
      r2r = pointToPoint.Install (routerSend.Get (i), routerReceive.Get (i));
      r2r.Get(1)-> SetAttribute ("ReceiveErrorModel", PointerValue (em));
      // Assign ip addresses
      Ipv4InterfaceContainer if2 = address2.Assign (r2r);
      address2.NewNetwork ();
      
      
      cout<<"link of "<<i<<" routers with bw "<<bwstring[i]<<", delay "<<delaystring[i]<<", and error rate "<<errorstring[i]<<endl;
      
        
      // Link from receiver to receiver's router
      pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("20Gbps"));
      receiverlink = pointToPoint.Install (node.Get (1), routerReceive.Get (i));
      // Assign ip addresses
      Ipv4InterfaceContainer if3 = address3.Assign (receiverlink);
      address3.NewNetwork ();
      // setup ip routes
      cmd_oss.str ("");
      cmd_oss << "rule add from " << if3.GetAddress (0, 0) << " table " << (i+1);
      LinuxStackHelper::RunIp (node.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add 10.3." << i << ".0/24 dev sim" << i << " scope link table " << (i+1);
      LinuxStackHelper::RunIp (node.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add default via " << if3.GetAddress (1, 0) << " dev sim" << i << " table " << (i+1);
      LinuxStackHelper::RunIp (node.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add 10.3."<<i<<".0/16 via " << if2.GetAddress (1, 0) << " dev sim1";
      LinuxStackHelper::RunIp (routerReceive.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());
      
      
      //link from sender's nuisance to sender's router
      
      pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("20Gbps"));
      snuisancelink = pointToPoint.Install (nuisanceSend.Get (i), routerSend.Get (i));
      Ipv4InterfaceContainer if4 = address4.Assign (snuisancelink);
      address4.NewNetwork ();
      
      //link from receiver's nuisance to receiver's router
      
      pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("20Gbps"));
      rnuisancelink = pointToPoint.Install (nuisanceReceive.Get (i), routerReceive.Get (i));
      Ipv4InterfaceContainer if5 = address5.Assign (rnuisancelink);
      address5.NewNetwork ();
      

      setPos (routerSend.Get (i), 40, i * 30, 0);
      setPos (routerReceive.Get (i), 80, i * 30, 0);
      setPos (nuisanceSend.Get (i), 40, 10+(i*30), 0);
      setPos (nuisanceReceive.Get (i), 80, 10+(i*30), 0);
      cout<<"done!"<<endl;
    }
	cout<<"looping done!"<<endl;
	
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  	#ifdef KERNEL_STACK
  	LinuxStackHelper::PopulateRoutingTables ();
	#endif
	cout<<"populate routing tables"<<endl;
	
	setPos (node.Get (0), 0, 30 * (path_num - 1) / 2, 0);
   	setPos (node.Get (1), 120, 30 * (path_num - 1) / 2, 0);
	
	
		
  // default route
  LinuxStackHelper::RunIp (node.Get (0), Seconds (0.1), "route add default via 10.1.0.2 dev sim0");
  LinuxStackHelper::RunIp (node.Get (1), Seconds (0.1), "route add default via 10.3.0.2 dev sim0");
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
  dce.AddArgument ("10.3.0.1");
  dce.AddArgument ("-i");
  dce.AddArgument ("3");
  dce.AddArgument ("-t");
  dce.AddArgument ("20");

  app1 = dce.Install (node.Get (0));
  app1.Start (Seconds (3));
  app1.Stop (Seconds (200));

  // Launch iperf server on node 0
  dce.SetBinary ("iperf");
  dce.ResetArguments ();
  dce.ResetEnvironment ();
  dce.AddArgument ("-s");
  app2 = dce.Install (node.Get (1));
  app2.Start (Seconds (1));
  
  
  for (int a=0; a<path_num; a++ ){
  	string b="";
  	b=convertInt(a);
        dce.SetBinary ("iperf");
        dce.ResetArguments ();
        dce.ResetEnvironment ();
        dce.AddArgument ("-c");
        dce.AddArgument ("10.5."+b+".1");
        dce.AddArgument ("-u");
        dce.AddArgument ("-b");
        dce.AddArgument (udpbwstring[a]+"m");
        cout<<"udp bandwidth for link "<<a<<" is "<<udpbwstring[a]<<endl;
        dce.AddArgument ("-t");
        dce.AddArgument ("20");

        app1 = dce.Install (nuisanceSend.Get (a));
        app1.Start (Seconds (3));
        app1.Stop (Seconds (200));

        // Launch iperf server on node 0
        dce.SetBinary ("iperf");
        dce.ResetArguments ();
        dce.ResetEnvironment ();
        dce.AddArgument ("-s");
        dce.AddArgument ("-u");
        app2 = dce.Install (nuisanceReceive.Get (a));
        app2.Start (Seconds (1));
  }
  
  
  pointToPoint.EnablePcapAll ("pikachu-mptcp");

  Simulator::Stop (Seconds (200));
  cout<<"start simulation"<<endl;
  Simulator::Run ();
  Simulator::Destroy ();
  cout<<"end simulation"<<endl;

  return 0;
}



