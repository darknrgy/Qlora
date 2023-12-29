#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Preferences.h>


#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  600  /* Time ESP32 will go to sleep (in seconds) */

// Wakeup interrupt pins
#define WAKEUP_PIN_LORA 13
#define WAKEUP_PIN_SENSE 25

// LoRa Pins
#define LORA_NSS 5
#define LORA_RST 14
#define LORA_DIO0 13

// LoRa Hops
#define LORA_HOPS 1

// Voltage Divider
#define VOLTAGE_READ_PIN0 A0
#define VOLTAGE_READ_PIN1 A1
#define VOLTAGE_DIVIDER_R1 1000000
#define VOLTAGE_DIVIDER_R2 220000

#define PREFERENCES_VERSION 5
#define PREFERENCES_NAMESPACE "qlora"
#define CONFIG Config::getInstance()

class LoRaProtocol;

class Config {
public:
	static Config& getInstance() {
		static Config instance;
		return instance;
	}

	static const size_t maxNameSize = 128;
	static const size_t maxIgnoreSize = 256;

	void setLora(LoRaProtocol* lora);
	void load();

	void toggleDebug();
	bool isDebug();

	void setBandwidth(long bw);
	long getBandwidth();	

	void toggleRelay();
	bool isRelay();

	void setPower(long power);
	long getPower();

	void setChannel(long channel);
	long getChannel();

	void setHops(long hops);
	long getHops();

	void setName(const char* name);
	const char* getName();

	void setIgnore(const char* ignore);
	const char* getIgnore();

	long getFrequency();

	void setDefaults();
	void save();
	long getChannelFrequency(long i);

	void getAllAsString(char* buffer, size_t bufferSize);

	Config(const Config&) = delete;
    void operator=(const Config&) = delete;

private:

	bool debug = true;
	long bandwidth;
	bool relay = true;
	long channel;
	long power;
	long hops;

	
	char name[maxNameSize];
	char ignore[maxIgnoreSize];

	Preferences prefs;

	LoRaProtocol* lora;

	long channels[128];
	Config();
};

#endif // CONFIG_H



/*
1: 902300000
2: 902500000
3: 902700000
4: 902900000
5: 903100000
6: 903300000
7: 903500000
8: 903700000
9: 903900000
10: 904100000
11: 904300000
12: 904500000
13: 904700000
14: 904900000
15: 905100000
16: 905300000
17: 905500000
18: 905700000
19: 905900000
20: 906100000
21: 906300000
22: 906500000
23: 906700000
24: 906900000
25: 907100000
26: 907300000
27: 907500000
28: 907700000
29: 907900000
30: 908100000
31: 908300000
32: 908500000
33: 908700000
34: 908900000
35: 909100000
36: 909300000
37: 909500000
38: 909700000
39: 909900000
40: 910100000
41: 910300000
42: 910500000
43: 910700000
44: 910900000
45: 911100000
46: 911300000
47: 911500000
48: 911700000
49: 911900000
50: 912100000
51: 912300000
52: 912500000
53: 912700000
54: 912900000
55: 913100000
56: 913300000
57: 913500000
58: 913700000
59: 913900000
60: 914100000
61: 914300000
62: 914500000
63: 914700000
64: 914900000
65: 915100000
66: 915300000
67: 915500000
68: 915700000
69: 915900000
70: 916100000
71: 916300000
72: 916500000
73: 916700000
74: 916900000
75: 917100000
76: 917300000
77: 917500000
78: 917700000
79: 917900000
80: 918100000
81: 918300000
82: 918500000
83: 918700000
84: 918900000
85: 919100000
86: 919300000
87: 919500000
88: 919700000
89: 919900000
90: 920100000
91: 920300000
92: 920500000
93: 920700000
94: 920900000
95: 921100000
96: 921300000
97: 921500000
98: 921700000
99: 921900000
100: 922100000
101: 922300000
102: 922500000
103: 922700000
104: 922900000
105: 923100000
106: 923300000
107: 923500000
108: 923700000
109: 923900000
110: 924100000
111: 924300000
112: 924500000
113: 924700000
114: 924900000
115: 925100000
116: 925300000
117: 925500000
118: 925700000
119: 925900000
120: 926100000
121: 926300000
122: 926500000
123: 926700000
124: 926900000
125: 927100000
126: 927300000
127: 927500000
128: 927700000
*/