#pikachu
=======

###pikachu is totally random name for iperf-mptcp scheme in NS-3 DCE.
to use:

1.  Download mptcp kernel, version 0.88 or 0.89 will do just fine. you can either use git clone or download the zip from here: https://github.com/multipath-tcp/mptcp

2.  follow Hajime's post on google+. Here I give you brief instruction:
1. add ns-3-dce (Direct Code Execution) code and merge to mptcp code
```      
% cd your/mptcp/folder
      % git remote add dce git://github.com/direct-code-execution/net-next-sim.git 
      % git fetch dce
      % git merge dce/sim-ns3-3.11.0-branch
```      
2. patch a bit manually
```      
% cat >> arch/sim/defconfig <<END
      CONFIG_MPTCP=y
      CONFIG_MPTCP_PM_ADVANCED=y
      CONFIG_MPTCP_FULLMESH=y
      CONFIG_MPTCP_NDIFFPORTS=y
      CONFIG_DEFAULT_FULLMESH=y
      CONFIG_DEFAULT_MPTCP_PM="fullmesh"
      CONFIG_TCP_CONG_COUPLED=y
      CONFIG_TCP_CONG_OLIA=y
      END
% make defconfig ARCH=sim
```
- or menuconfig to enable these options (I prefer to use this to enable all TCP congestion controls and path managers. make sure all of required packges are installed)
```      	
% make menuconfig ARCH=sim
```
3. build kernel (as a shared library)
```      
% make library ARCH=sim
```      
   If everything is going well, you can try to use it over ns-3
4. build ns-3 related tools
```      
`% make testbin -C arch/sim/test
```
3.  (to obtain visualizer) after building, you have to erase folder pybindgen in arch/sim/test/buildtop/source folder and go to ns-3-dce/bindings/python. edit wscript file of pybindgen version to "0, 17, 0, 868". same goes for ns-3-dev-dce folder. Then go to arch/sim/test/bake folder. Open bakeconf.xml file to any text editor. find pybindgen packages there. There will be several xml tree of pybindgen. Get the top one, and it should look like this:
```
<module name="pybindgen">
      <source type="bazaar">
	<attribute name="url" value="https://launchpad.net/pybindgen"/>
	<attribute name="revision" value="revno:868"/>
```
##### NOTE: I had tried other version, most of them aren't working well. if they had made the patch, you are welcome to change it.

4.  go to your mptcp folder, rebuild it by using command on 2. 4)
5.  clone this git inside arch/sim/test/builtop/source/ns-3-dce/myscript folder
    - after cloning, you should move note.xml to ns-3-dce folder. this xml is the source of configuration file in pikachu
6.  in ns-3-dce folder. and enter this to build pikachu:
```    
% ./waf
```
run it!
```    
% ./waf --run pikachu --visualize
```
any other configuration (adding path, tuning bw, etc) goes to note.xml


```
topology:
                NUISANCE                        NUISANCE
                    |                               |
            |--- R SENDER 1 ----------------- R RECEIVER 1 ---|
            |                                                 |
========    |--- R SENDER 2 ----------------- R RECEIVER 2 ---|   ===========
 SENDER  ---|       |                               |         |---  RECEIVER
========    |   NUISANCE                        NUISANCE      |   ===========
            |                                                 |
            |                                                 |
            |                                                 |
            |--- R SENDER N ----------------- R RECEIVER 2 ---|
                    |                               |
                NUISANCE                        NUISANCE
```
