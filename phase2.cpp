// Phase2Version2.cpp




#include <iostream>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <random>
#include <iomanip>
using namespace std;


bool debug = false;

//Class Definitions
class Node{
public:
	int ID;
	double nextPacketArrivalTime;
	Node(int ID, double nextArrival);	
};

Node::Node(int nextID, double nextArrival){
	ID = nextID;
	nextPacketArrivalTime = nextArrival;
}

class Packet{
public:
	double arrivalTime;
	int destination;
	int packetSize;
	Packet(double arrivalTime, int destination, int packetSize);
};

Packet::Packet(double arrivingAt, int goingTo, int size){
	arrivalTime = arrivingAt;
	destination = goingTo;
	packetSize = size;
}

//Methods

//Determines next host (supports looping)
int nextHost(int curTokenLocation, int numHosts){
	curTokenLocation++;
	if (curTokenLocation >= numHosts){
		curTokenLocation = 0;
		return curTokenLocation;
	}
	else{
		return curTokenLocation;
	}
}

double genRandArrival(double startTime, double rate){
	double u;
	u = drand48();
	return startTime + ((-1/rate)*log(1-u));	
	//PLACEHOLDER!
	//return 0.003;
	//return random arrival (startTime + randomGen'd time)
}

int genRandSize(){ //uniformly random gen [63, 1518]
	std::random_device rd; // obtain a random number from hardware
	std::mt19937 rng(rd()); // seed the generator
	std::uniform_int_distribution<int> uni(64, 1518); // define the range

	int random = uni(rng);
	return random;
	//return random size
}

int genRandDest(int numHosts, int curTokenLocation){	//uniformly random gen [0, numHost]
	std::random_device rd; // obtain a random number from hardware
	std::mt19937 rng(rd()); // seed the generator
	std::uniform_int_distribution<int> uni(0, numHosts); // define the range

	int random = uni(rng);
	//account for dest being source
	while(random == curTokenLocation){
		random = uni(rng);
	}
	return random;
}

int distanceToDest(int numHosts, int startingLocation, int destination){
	//note: indexing by 0!
	if (destination == 0 && startingLocation == numHosts - 1){
		//means starting location is "last" node and dest is "start" -> dist == 1
		return 1;
	}

	int result = destination - startingLocation;

	//means need to loop
	if (result < 0){
		int temp = (numHosts - startingLocation) + destination;
		return temp;
	}
	//result is positive
	else{
		return result;
	}
	//returns the number of nodes that must be traversed from startingLocation
	//to destination
	//NOTE: needed because nodes wrap
}
//function to retrieve which node has next arrival time in entire system
int getMin(vector<Node*> globalStorage){
	if(debug == true){cout << "Entered getmin function"<< endl;}
	int min = globalStorage[0]->nextPacketArrivalTime;
	int node = 0;
	int x;	
	if(debug == true){cout << "getmin(), executing for loop"<< endl;}
	for(x = 1; x < globalStorage.size(); x++){
		if(min > globalStorage[x]->nextPacketArrivalTime){
			min = globalStorage[x]->nextPacketArrivalTime;
			node = x;
		}
	}
	if(debug == true){cout << "returning from getmin()" << endl;}
	//return min;
	return node;
}

int main()
{		
	//constants
	const double transferRate = 0.00000001;//1sec/100Mbps 1 / (100 * 10 ^ 6);
	const double propRate = 0.00001;	//10us 10 * 10 ^ -6	
	const double byteToBitConversion = 8; // 8 bits / 1 byte
	
	//User defined
	int numHosts = 25;
	double lambda = 0.01;

	cout << "Enter number of hosts: ";
	cin >> numHosts;
	cin.ignore(1000, 10);
	cout << "Enter lambda: ";
	cin >> lambda;
	cin.ignore(1000, 10);
	//Token
	int curTokenLocation = 0;

	//Frame
	vector<Packet*> frame;

	//data structures
	vector<Node*> globalStorage;	//holds all hosts in the system

	//Time
	double currentSystemTime = genRandArrival(0,lambda);

	//create first node, always has first arrival
	Node * first = new Node(0, currentSystemTime);
	globalStorage.push_back(first);


	//create the remaining nodes
	for (int j = 1; j < numHosts; j++){
		//not sure about this
		double test = currentSystemTime;
		test += genRandArrival(currentSystemTime,lambda);
		assert(test > currentSystemTime);
		Node * node = new Node(j, test);
		globalStorage.push_back(node);
	}

	//Stats variables
	double totalDelay = 0;
	int totalTransmittedBytes = 0;
	int numberPacketSent = 0;
	
	int numberAddSent = 0;
	
	if(debug == true){cout << "entering for loop" << endl;}

	//Main loop
	for (int k = 0; k < 100000; k++){
		if(debug == true){cout << " " <<endl;}
		if(debug == true){cout << "Entered for loop" << endl;}
		
		//Determine if CurrentTime in system is smaller than all arrivals times'
		//if it is, adjust currentTime to the smallest arrival time
		int nextNode = getMin(globalStorage);
		if(debug == true){cout << "just called getMin(globalStorage)" << endl;}
		double minimumNextArrival = globalStorage[nextNode]->nextPacketArrivalTime;
		if(debug == true){
			cout << "nextNode = " << nextNode << endl;
			cout << "minimumNextArrival = " << minimumNextArrival << endl;
			cout <<  "currentSystemTime = " << currentSystemTime << endl;
		}

		
		if(currentSystemTime < minimumNextArrival){
			currentSystemTime = minimumNextArrival;
			curTokenLocation = nextNode;
		}
		if(debug == true){cout << "after currentTime vs arrival time check" << endl;}
		
		//if node has packets to send (sends does full send cycle)
		if (globalStorage[curTokenLocation]->nextPacketArrivalTime <= currentSystemTime){
			//add current packet to the frame
			int randSize = genRandSize();
			totalTransmittedBytes += randSize;
			if(debug == true){cout << "randSize = " << randSize << endl;}
			frame.push_back(new Packet(globalStorage[curTokenLocation]->nextPacketArrivalTime, genRandDest(numHosts,curTokenLocation), randSize));
			
			numberPacketSent++;
			
			//generate next arrival time
			globalStorage[curTokenLocation]->nextPacketArrivalTime = genRandArrival(globalStorage[curTokenLocation]->nextPacketArrivalTime,lambda);
			//keep generating packets to send until arrival time exceeds current time
			while (globalStorage[curTokenLocation]->nextPacketArrivalTime < currentSystemTime){
				randSize = genRandSize();
				totalTransmittedBytes += randSize;
				if(debug == true){cout << "randSize = " << randSize << endl;}
				frame.push_back(new Packet(globalStorage[curTokenLocation]->nextPacketArrivalTime, genRandDest(numHosts,curTokenLocation), randSize));
				//generate next arrival time
				globalStorage[curTokenLocation]->nextPacketArrivalTime = genRandArrival(globalStorage[curTokenLocation]->nextPacketArrivalTime,lambda);
				
				numberPacketSent++;
				numberAddSent++;
			}
			//now frame is proper size, next update delay times

			//double longestTime = 0;
			double totalTransTime = 0;
			
			int frameSize = 0;

			for (int i = 0; i < frame.size(); i++){

				double temp;
				int numHostsToDest = distanceToDest(numHosts, curTokenLocation, frame[i]->destination);

				//add queuing delay
				double timeDiff = currentSystemTime - frame[i]->arrivalTime;
				totalDelay += timeDiff;

				//add/calc transmission delay
				frameSize += frame[i]->packetSize;
				double timeToNextHost = frame[i]->packetSize * byteToBitConversion * transferRate * numHostsToDest;
				totalDelay += timeToNextHost;

				//cout << "Time to next host value = " << timeToNextHost << endl;
				
				//add propagation delay
				double propdelay = propRate * numHostsToDest;
				totalDelay += propdelay;

				
				
				
				
			}
			
			//calculate roundtrip time of frame
			totalTransTime = frameSize * byteToBitConversion * transferRate * numHosts;
			totalTransTime += propRate * numHosts;
			
			
			//now clear frame
			frame.erase(frame.begin(), frame.end());


			//finished sending packets, looped back to original destination, update current time
			//and pass token to next node
			//currentSystemTime = longestTime;
			
			//update current system time to end of roundtrip transmission time and move token to next host
			currentSystemTime += totalTransTime;			
			curTokenLocation = nextHost(curTokenLocation, numHosts);

		}
		//doesnt have packets to send right now
		//just forward token
		else{
			if(debug == true){cout << "Entered ELSE" << endl;}
			curTokenLocation = nextHost(curTokenLocation, numHosts);
		}
		if(debug == true){
			cout << "End of an iteration" << endl;
			cout << "Current total delay = " << totalDelay << endl;
			cout << "Current total Transmitted Bytes " << totalTransmittedBytes << endl;
			cout << "Current number of packets sent = " << numberPacketSent << endl;
			cout << " " << endl;
		}
	}

	//double averagePacketDelay = totalDelay / currentSystemTime;
	double averagePacketDelay = totalDelay / numberPacketSent;
	double throughput = totalTransmittedBytes / currentSystemTime;
	//double throughput = totalTransmittedBytes / numberPacketSent;
	cout << "********************" << endl;
	cout << "Simulation Results" << endl;
	cout << "Number of packets transmitted " << numberPacketSent << endl;
	cout << "Number of Bytes Transmitted: " << totalTransmittedBytes << endl;
	cout << "Ending Time in System: " << currentSystemTime << endl;
	cout << "Throughput: " << setprecision(4) << fixed << throughput << endl;
	cout << "Total Delay: " << totalDelay << endl;
	cout << "Average Packet Delay: " << setprecision(4) << fixed << averagePacketDelay << endl;
	cout << "********************" << endl;
	for(std::vector<Packet*>::iterator it = frame.begin(); it != frame.end(); ++it)
		delete (*it);
	frame.clear();
	for(std::vector<Node*>::iterator it = globalStorage.begin(); it != globalStorage.end(); ++it)
		delete (*it);
	globalStorage.clear();
	
	cout << "End of program" << endl;
	if(debug == true){cout << "DEBUG: number of xtra generated " << numberAddSent << endl;}
	//char final;
	//cin >> final;

	return 0;
}