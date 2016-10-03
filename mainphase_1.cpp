//***************************************************************************
//Comments:
//length var in paper == current queue.size();
//mu and lambda are the 'rate' for negativeExpo input
//
//Stats variables:
//		serverBusyTime = total time server is actively processing packets
//		areaUnderCurve = total area under queueSize vs time graph 
//		packetsDropped = number of dropped packets
//
//
//Currently working on actual calculations...Pretty sure something isn't right
//because results seems way off... I hope I didn't mess something up
//
//
//Update Set up a way to have an infinite buffer
//		 Idea: have input of '-1' == infinite buffer (easy to check against)?
//
//
//STATUS: Compiles and produces output, but I believe the output is incorrect :(
//
//***************************************************************************


#include <iostream>
#include <stdlib.h>
#include <list>
#include <queue>
#include <time.h>
#include <cmath>
using namespace std;
const int EVENT_ARRIVAL = 0;
const int EVENT_DEPARTURE = 1;
bool DEBUG_ON = true;
class Event{
	int id;
	public:
	double eventTime;
	int eventType; //Arrival? Departure?
	//Event* prevEvent; //may not be needed...
	//Event* nextEvent; //"                  "
	Event(); //constructor
	Event(double, int, int);
	bool operator<(Event*); //this is for the sort function for GEL
};

class Packet{
	public:
	double serviceTime;
	Packet();
	Packet(double);
};

double negativeExpo(double rate){
	double u;
	u = drand48();
	return ((-1/rate)*log(1-u));
};

int main(){
	//Variables TBD by user
	double lambda; 	//rate at which packets arrive
	double mu; 		//rate for service time of packets
	int maxBuffer;	//largest amount of packets allowed
	
	//Variables needed for statstics
	int packetsDropped = 0;
	double serverBusyTime = 0;
	double areaUnderCurve = 0;
	
	//variables for debugging purposes
	double lastTime = -1;
	
	cout<<"Enter an arrival rate (lambda):";
	cin >> lambda;
	cin.ignore(1000, 10);
	cout<<"Enter a service rate (mu):";
	cin >> mu;
	cout<<"Enter the maximum number of packets allowed in the buffer: ";
	cin >> maxBuffer;
	
	//Create the Main Data Structures
	cout<<"Creating Global Event List...";
	list<Event*> globalEventList;
	cout<<"...Done!\n";
	cout<<"Creating Packet Queue...";
	queue<Packet*> theQueue;
	cout<<"...Done!\n";
	double currentTime = 0;
	list<Event*>::iterator it;
	if(DEBUG_ON) lastTime = currentTime;
	
	double priorEventTime = 0; //for calculating delta for mean queue length
	//1st packet doesnt arrive at time = 0
	double randomNumLambda = negativeExpo(lambda);
	globalEventList.push_front(new Event(currentTime + randomNumLambda,EVENT_ARRIVAL,1));
	int eventCount = 2;	 
	Event* currentEvent;
	//TODO:BUG, Status == Fixed
	/* 	Step 2, need to push the currently being processed packet into the Queue
		if not done, then if queue is initially empty, Arrival_1 followed by another arrival_2
		followed by departure_1, the queue will never be populated
		TLDR: if queue == empty, push the arriving packet from the arrival event into the queue
		to represent the router as busy
	*/
	//The following is the structure for our "clock," since we won't be using system time
	for(int i = 0; i < 100000; i++){
		//1: get the first event from the GEL, schedule next event
		currentEvent = globalEventList.front();
		randomNumLambda = negativeExpo(lambda); //this will be replaced by our negative exponential distribution
		double randomNumMu = negativeExpo(mu); //this will be replaced by our negative exponential distribution		
		
		//2: if the event is an arrival then process-arrival-event
		if(currentEvent->eventType == EVENT_ARRIVAL){
			currentTime = currentEvent->eventTime;
			if(DEBUG_ON && lastTime > currentTime) cout<<"ERROR: Timelapse error during ARRIVAL_EVENT\n";
			if(DEBUG_ON) lastTime = currentTime;
			//inserting next newly arriving packet since one just arrived 
			for(it = globalEventList.begin(); (*it)->eventTime > currentTime+randomNumLambda ; ++it){
				//Iterating . . .
			}
			globalEventList.insert(it, new Event(currentTime + randomNumLambda, EVENT_ARRIVAL, eventCount));
			eventCount++;
			
			//now determing what is happening with current event (@ head of GEL)			
			if(theQueue.empty() == true){//insert being processed packet into queue, push its departure time into GEL
				theQueue.push(new Packet(randomNumMu));
							for(it = globalEventList.begin(); (*it)->eventTime > currentTime+randomNumLambda ; ++it){
				//Iterating . . .
			}
			globalEventList.insert(it, new Event(currentTime + randomNumLambda, EVENT_ARRIVAL, eventCount));
				eventCount++;
			} 
			else{
				if(maxBuffer == -1){// then maxBuffer = infinite
					theQueue.push(new Packet(randomNumMu));					
				}
				else{//Buffer is finite				
					if(theQueue.size() - 1 < maxBuffer){
						theQueue.push(new Packet(randomNumMu));
					}
					else{
						packetsDropped++;
					}
				}
			}
			
		}
		//3: else it must be a departure event, use process-service-completion
		else{//need to add the time server was active to total for statistics
			currentTime = currentEvent->eventTime;		
			if(DEBUG_ON && lastTime > currentTime) cout<<"ERROR: Timelapse error during DEPARTURE_EVENT\n";
			if(DEBUG_ON) lastTime = currentTime;
			serverBusyTime += theQueue.front()->serviceTime;
			theQueue.pop();
			if(theQueue.size() > 0){
							for(it = globalEventList.begin(); (*it)->eventTime > currentTime+randomNumLambda ; ++it){
				//Iterating . . .
			}
			globalEventList.insert(it, new Event(currentTime + randomNumLambda, EVENT_ARRIVAL, eventCount));
			}
		}
		
		//finally remove the current head of GEL since its been processed
		globalEventList.pop_front();
		
		//update areaUnderCurve for mean queue length calc
		areaUnderCurve += theQueue.size() * (currentTime - priorEventTime);
		priorEventTime = currentTime;
	}	
	//print out statistical results
	cout << "Total Server busy time: " << serverBusyTime << endl;
	cout << "Ending time of simulation: " << currentTime << endl;
	cout << "Area Under Curve: " << areaUnderCurve << endl;
	cout << "Calculating Utilization...";	
	double Utilization = serverBusyTime / currentTime;
	cout << "...Done!" << endl;
	cout << "Calculating Mean Queue Length...";
	double meanQueueLength = areaUnderCurve/currentTime;
	cout << "...Done!" << endl;
	
	cout << endl;
	cout << "********************" << endl;
	
	cout << "Simulation Results for arrival rate = " << lambda << ", service rate = " << mu;
	cout << ", and maximum buffer size of = " << maxBuffer << endl;
	cout << "Server Utilization: " << Utilization << endl;
	cout << "Mean Queue Length: " << meanQueueLength << endl;
	cout << "Number of Packets Dropped: " << packetsDropped << endl;
	cout << "********************" << endl;
	cout << endl;
}
Event::Event(){
}

Event::Event(double time, int type, int ID){ //Event constructor
	eventTime = time;
	eventType = type;
	id = ID;
}

bool Event::operator<(Event* event){
	if(this->eventTime < event->eventTime){
		return true;
	}
	else{
		return false;
	}
}

Packet::Packet(){
}

Packet::Packet(double servTime){
	serviceTime = servTime;
}