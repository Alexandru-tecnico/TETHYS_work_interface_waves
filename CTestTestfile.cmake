# CMake generated Testfile for 
# Source directory: /home/alexandru/Desktop/dissert/sim/TETHYS-Graphene-Hydrodynamic-Simulation-master
# Build directory: /home/alexandru/Desktop/dissert/sim/TETHYS-Graphene-Hydrodynamic-Simulation-master/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test2D "TETHYS_2D" "2DTESTparameters.ini")
set_tests_properties(test2D PROPERTIES  _BACKTRACE_TRIPLES "/home/alexandru/Desktop/dissert/sim/TETHYS-Graphene-Hydrodynamic-Simulation-master/CMakeLists.txt;53;add_test;/home/alexandru/Desktop/dissert/sim/TETHYS-Graphene-Hydrodynamic-Simulation-master/CMakeLists.txt;0;")
add_test(testElec2D "TETHYS_ELEC_2D" "hdf5_2D_S=21.00vF=10.50vis=0.010odd=0.002l=0.001wc=0.00therm=0.45.h5")
set_tests_properties(testElec2D PROPERTIES  _BACKTRACE_TRIPLES "/home/alexandru/Desktop/dissert/sim/TETHYS-Graphene-Hydrodynamic-Simulation-master/CMakeLists.txt;55;add_test;/home/alexandru/Desktop/dissert/sim/TETHYS-Graphene-Hydrodynamic-Simulation-master/CMakeLists.txt;0;")
