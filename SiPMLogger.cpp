// notes: dialout group, dmesg | tail, usb-devices
// todo: add multiple devices, custom filename
// ftdi: vendor: 0x0403, product: 0x6001
// compile: g++ SiPM_Logger.cpp -o SiPM_Logger -lusb

// A/B Seite:
// T:  Bus=01 Lev=05 Prnt=124 Port=03 Cnt=02 Dev#=126 Spd=12  MxCh= 0
// D:  Ver= 2.00 Cls=00(>ifc ) Sub=00 Prot=00 MxPS= 8 #Cfgs=  1
// P:  Vendor=0403 ProdID=6001 Rev=06.00
// S:  Manufacturer=FTDI
// S:  Product=USB <-> Serial Converter
// S:  SerialNumber=FT1L0EZJ
// C:  #Ifs= 1 Cfg#= 1 Atr=80 MxPwr=100mA
// I:  If#= 0 Alt= 0 #EPs= 2 Cls=ff(vend.) Sub=ff Prot=ff Driver=ftdi_sio

// C/D Seite:
// Bus=01 Lev=05 Prnt=124 Port=02 Cnt=01 Dev#=125 Spd=12  MxCh= 0
// D:  Ver= 2.00 Cls=00(>ifc ) Sub=00 Prot=00 MxPS= 8 #Cfgs=  1
// P:  Vendor=0403 ProdID=6001 Rev=06.00
// S:  Manufacturer=FTDI
// S:  Product=FT232R USB UART
// S:  SerialNumber=A524YVP8
// C:  #Ifs= 1 Cfg#= 1 Atr=a0 MxPwr=90mA
// I:  If#= 0 Alt= 0 #EPs= 2 Cls=ff(vend.) Sub=ff Prot=ff Driver=ftdi_sio

#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <cstdlib>
#include <fstream>
#include <time.h>
#include <iomanip>
#include <ctime>
#include <signal.h>
#include <usb.h>
#include "SiPMLogger.h"
#include "TFile.h"
#include "TTree.h"

volatile sig_atomic_t stop = 0;

double GetVoltage(const char *port = "/dev/ttyUSB0") {
	double voltage = SendCommand("HGV", "", port);
	return voltage;
}

double GetTemperature(const char *port = "/dev/ttyUSB0") {
	double voltage = SendCommand("HGT", "", port);
	return voltage;
}

double GetCurrent(const char *port = "/dev/ttyUSB0") {
	double voltage = SendCommand("HGC", "", port);
	return voltage;
}
/*
 * catch Ctrl+C and then exit while loop
 * so we can close the files nicely
 */
void exit_handler(int signum){
	stop = 1;
}
int main(int argc, char* argv[]) {
	double V1, I1, T1, V2, I2, T2 = 0;
	int N_seconds = 5;
	const char* port_1 = "/dev/ttyUSB0";
	const char* port_2 = "/dev/ttyUSB1";
	bool single_SiPM = true;
	float apply_voltage = 52.7;
	signal(SIGINT, exit_handler);

	// // debugging
	// struct usb_bus *bus;
	// struct usb_device *dev;
	// usb_init();
	// usb_find_busses();
	// usb_find_devices();
	// for (bus = usb_busses; bus; bus = bus->next) {
	//   for (dev = bus->devices; dev; dev = dev->next){
	//       printf("Trying device %s/%s\n", bus->dirname, dev->filename);
	//       printf("\tID_VENDOR = 0x%04x\n", dev->descriptor.idVendor);
	//       printf("\tID_PRODUCT = 0x%04x\n", dev->descriptor.idProduct);
	//     }
	// }
	if (argv[argc-1] == (std::string)"--help") {
		std::cout << "Usage: ./SiPM_Logger filename number_SiPMs apply_voltage" << std::endl;
		return 0;
	}
	else {
		std::cout << "Log filename: " << argv[1] << std::endl;
		if (atoi(argv[2]) == 1) {
			std::cout << "Reading one SiPM." << std::endl;
		}
		if (atoi(argv[2]) == 2) {
			std::cout << "Reading two SiPMs." << std::endl;
			single_SiPM = false;
		}
		if(argc == 4) {
			//set voltage, limit to between 0 and 54, but the minimum seems to be 39.237V
			if(atof(argv[3]) < 54. && atof(argv[3]) > 0.) {
				apply_voltage = atof(argv[argc-1]);
			} // if not set, then use default 52.7V
		}
	}
	std::cout << std::endl;

	TFile *fOutput = new TFile(((std::string)argv[1] + ".root").c_str(), "RECREATE");
	TDatime timestamp;
	Float_t voltage, current, temp;
	TTree *tree = new TTree("tree", "tree");
	tree->Branch("timestamp", &timestamp);
	tree->Branch("voltage", &voltage);
	tree->Branch("current", &current);
	tree->Branch("temp", &temp);

	std::ofstream myfile;
	myfile.open(((std::string)argv[1] + ".txt").c_str());
	if (single_SiPM)
		myfile << "time\ttemperature\tvoltage\tcurrent\n";
	else
		myfile << "time\ttemperature1\tvoltage1\tcurrent1\ttemperature2\tvoltage2\tcurrent2\n";

	time_t t_now = time(0);
	unsigned int i_sample = 0;

	//set voltage
	SendHBV(std::to_string(apply_voltage).c_str() );

	while(!stop) {
		if ((time(0) - t_now) == N_seconds) {
			auto t = std::time(nullptr);
			auto tm = *std::localtime(&t);

			V1 = GetVoltage(port_1);
			I1 = GetCurrent(port_1);
			T1 = GetTemperature(port_1);
			std::cout << i_sample << std::endl;
			std::cout << "Date / Time:\t" << std::put_time(&tm, "%d/%m/%Y %H:%M:%S") << std::endl;
			std::cout << "Voltage_1:\t" << V1 << " V" << std::endl;
			std::cout << "Current_1:\t" << I1 << " A"<< std::endl;
			std::cout << "Temperature_1:\t" << T1 << " C" << std::endl;

			if (!single_SiPM) {
				V2 = GetVoltage(port_2);
				I2 = GetCurrent(port_2);
				T2 = GetTemperature(port_2);
				std::cout << "Voltage_2:\t" << V2 << " V" << std::endl;
				std::cout << "Current_2:\t" << I2 << " A"<< std::endl;
				std::cout << "Temperature_2:\t" << T2 << " C" << std::endl;
			}
			std::cout << std::endl;
			/*
			 * write to <filename>.txt log file
			 */
			myfile << std::put_time(&tm, "%d/%m/%Y %H:%M:%S") << "\t" << std::to_string(T1) << "\t"
				<< std::to_string(V1) << "\t" << std::to_string(I1);
			if (!single_SiPM) {
				myfile << "\t" << std::to_string(T2) << "\t"
					<< std::to_string(V2) << "\t" << std::to_string(I2);
			}
			myfile << std::endl;
			t_now = time(0);
			i_sample++;

			/*
			 * fill root TTree
			 */
			timestamp = TDatime();
			voltage = V1;
			current = I1;
			temp = T1;
			tree->Fill();
		}
	}
	fOutput->Write();
	fOutput->Close();

	myfile.close();
	return 0;
}
