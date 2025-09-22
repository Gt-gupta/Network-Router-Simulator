#include <iostream>
#include <queue>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <map>
#include <numeric>

using namespace std;

const int NUM_PORTS = 8;        // Number of input/output ports
const int BUFFER_SIZE = 64;     // Maximum buffer size for each queue
const int MAX_PACKETS = 2000;   // Maximum number of packets to simulate

struct Packet {
    int id;
    int priority;                // Higher priority means higher precedence
    int arrivalTime;             // Time when the packet arrives
    int processingTime;          // Time taken to process the packet
    int outputPort;              // Port to which the packet is destined
    int waitingTime;             // Time spent waiting in the queue
    int turnaroundTime;          // Total time in the system
};

// Packet generation functions
Packet generatePacketUniform(int id, int currentTime) {
    Packet pkt;
    pkt.id = id;
    pkt.priority = rand() % 5;  // Priorities from 0 to 4
    pkt.arrivalTime = currentTime;
    pkt.processingTime = rand() % 10 + 1;  // Processing time between 1 and 10
    pkt.outputPort = rand() % NUM_PORTS;  // Randomly assign to one of the output ports
    pkt.waitingTime = 0;  // Initialize
    pkt.turnaroundTime = 0;  // Initialize
    return pkt;
}

Packet generatePacketNonUniform(int id, int currentTime) {
    Packet pkt;
    pkt.id = id;
    pkt.priority = rand() % 5;  // Priorities from 0 to 4
    pkt.arrivalTime = currentTime;
    pkt.processingTime = rand() % 10 + 1;
    pkt.outputPort = rand() % NUM_PORTS;
    pkt.waitingTime = 0;
    pkt.turnaroundTime = 0;

    // Assign higher priority to certain ports (e.g., port 0 and 1 receive higher priority)
    if (id % 2 == 0) {
        pkt.priority = 4;  // Highest priority
    }
    return pkt;
}

Packet generatePacketBursty(int id, int currentTime, bool burst) {
    Packet pkt;
    pkt.id = id;
    pkt.priority = rand() % 5;
    pkt.arrivalTime = currentTime;
    pkt.processingTime = rand() % 10 + 1;
    pkt.outputPort = rand() % NUM_PORTS;
    pkt.waitingTime = 0;
    pkt.turnaroundTime = 0;

    // Simulate bursty traffic
    if (burst) {
        pkt.priority = 4;  // High priority burst traffic
    }
    return pkt;
}

// Priority scheduling helper function

// Priority scheduling helper function
vector<Packet> helperPriority(vector<queue<Packet>>& inputQueues) {
    map<int, vector<Packet>> mp;  // Map to group packets by their output port

    // Fill the map with packets from the input queues
    for (int i = 0; i < NUM_PORTS; ++i) {
        if (!inputQueues[i].empty()) {
            Packet pkt = inputQueues[i].front();
            mp[pkt.outputPort].push_back(pkt);
        }
    }

    vector<Packet> ret;  // To store the packets that will be scheduled

    // Process the packets for each output port
    for (auto x : mp) {
        // If only one packet for this output port
        if (x.second.size() == 1) {
            ret.push_back(x.second[0]);  // Add the packet to the result

            // Remove the packet from the corresponding input queue
            for (int i = 0; i < NUM_PORTS; i++) {
                if (!inputQueues[i].empty() && inputQueues[i].front().id == x.second[0].id) {
                    inputQueues[i].pop();
                    break;
                }
            }
        } 
        // If there are multiple packets for this output port
        else {
            Packet highestPriorityPacket;
            highestPriorityPacket.priority = -1;  // Lowest possible priority to start
            Packet selectedPacket;  // This will hold the selected packet

            for (const auto& pkt : x.second) {
                // Find the highest priority packet
                if (pkt.priority > highestPriorityPacket.priority) {
                    highestPriorityPacket = pkt;
                    selectedPacket = pkt;  // Keep track of the selected packet
                }
            }

            // Remove the highest priority packet from the corresponding input queue
            for (int i = 0; i < NUM_PORTS; i++) {
                if (!inputQueues[i].empty() && inputQueues[i].front().id == selectedPacket.id) {
                    inputQueues[i].pop();
                    break;
                }
            }
            ret.push_back(selectedPacket);  // Add the selected packet to the result
        }
    }
    return ret;  // Return the packets scheduled
}

// Weighted Fair Queuing Helper Function
vector<Packet> helperWFQ(vector<queue<Packet>>& inputQueues, vector<int>& weights) {
    map<int, vector<Packet>> mp;  // Map to group packets by their output port

    // Fill the map with packets from the input queues
    for (int i = 0; i < NUM_PORTS; ++i) {
        if (!inputQueues[i].empty()) {
            Packet pkt = inputQueues[i].front();
            mp[pkt.outputPort].push_back(pkt);
        }
    }

    vector<Packet> ret;  // To store the packets that will be scheduled

    // Process the packets for each output port
    for (auto x : mp) {
        // If only one packet for this output port
        if (x.second.size() == 1) {
            ret.push_back(x.second[0]);  // Add the packet to the result

            // Remove the packet from the corresponding input queue
            for (int i = 0; i < NUM_PORTS; i++) {
                if (!inputQueues[i].empty() && inputQueues[i].front().id == x.second[0].id) {
                    inputQueues[i].pop();
                    break;
                }
            }
        } 
        // If there are multiple packets for this output port
        else {
            Packet selectedPacket;
            int maxWeight = -1;
            for (const auto& pkt : x.second) {
                // Check if the weight is higher than previously selected
                int portIndex = pkt.outputPort;
                if (weights[portIndex] > maxWeight) {
                    maxWeight = weights[portIndex];
                    selectedPacket = pkt;  // Keep track of the selected packet
                }
            }

            // Remove the selected packet from the corresponding input queue
            for (int i = 0; i < NUM_PORTS; i++) {
                if (!inputQueues[i].empty() && inputQueues[i].front().id == selectedPacket.id) {
                    inputQueues[i].pop();
                    break;
                }
            }
            ret.push_back(selectedPacket);  // Add the selected packet to the result
        }
    }
    return ret;  // Return the packets scheduled
}

// Round Robin Helper Function
vector<Packet> helperRoundRobin(vector<queue<Packet>>& inputQueues, int& lastScheduledPort) {
    vector<Packet> ret;  // To store the packets that will be scheduled
    for (int i = 0; i < NUM_PORTS; ++i) {
        lastScheduledPort = (lastScheduledPort + 1) % NUM_PORTS;  // Rotate through ports
        if (!inputQueues[lastScheduledPort].empty()) {
            ret.push_back(inputQueues[lastScheduledPort].front());  // Add the packet to the result
            inputQueues[lastScheduledPort].pop();  // Remove from queue
            break;  // Only one packet per round-robin cycle
        }
    }
    return ret;  // Return the packets scheduled
}

// iSLIP Helper Function
vector<Packet> helperiSLIP(vector<queue<Packet>>& inputQueues, vector<int>& pointers) {
    vector<Packet> ret;  // To store the packets that will be scheduled
    for (int i = 0; i < NUM_PORTS; ++i) {
        int currentPort = (pointers[i] + i) % NUM_PORTS;  // Calculate the current port

        if (!inputQueues[currentPort].empty()) {
            ret.push_back(inputQueues[currentPort].front());  // Add the packet to the result
            inputQueues[currentPort].pop();  // Remove from queue
            pointers[i] = (pointers[i] + 1) % NUM_PORTS;  // Advance the pointer
            break;  // Only one packet per iSLIP cycle
        }
    }
    return ret;  // Return the packets scheduled
}

int main() {
    srand(time(0));

    // Buffers for each input port
    vector<queue<Packet>> inputQueues(NUM_PORTS);
    vector<queue<Packet>> outputQueues(NUM_PORTS);

    // Weights for WFQ
    vector<int> wfqWeights(NUM_PORTS, 3);  // Each port has equal weight initially
    int totalWeight = accumulate(wfqWeights.begin(), wfqWeights.end(), 0);

    // Pointers for iSLIP algorithm
    vector<int> iSLIPPointers(NUM_PORTS, 0);

    int lastScheduledPort = 0;  // For round-robin
    int currentTime = 0;
    int packetCount = 1;

    // Track metrics
    int processedPackets = 0;  
    int totalWaitingTime = 0;
    int totalTurnaroundTime = 0;
    int droppedPackets = 0;  
    int totalGeneratedPackets = 0;  

    // User input for scheduling algorithm
    int schedulingChoice;
    cout << "Select Scheduling Algorithm: \n1. Priority\n2. Weighted Fair Queuing (WFQ)\n3. Round Robin\n4. iSLIP\n";
    cin >> schedulingChoice;

    // Run the simulation for MAX_PACKETS packets
    while (packetCount < MAX_PACKETS) {
        bool burst = rand() % 10 < 2;  // Randomly generate bursts for bursty traffic

        // Generate packets based on the traffic pattern
        for (int i = 0; i < NUM_PORTS; ++i) {
            
                Packet pkt;
                int trafficChoice = rand()%3+1;
                totalGeneratedPackets++;  // Increment the count of generated packets
                if (trafficChoice == 1) {
                    pkt = generatePacketUniform(packetCount++, currentTime);
                } else if (trafficChoice == 2) {
                    pkt = generatePacketNonUniform(packetCount++, currentTime);
                } else {
                    pkt = generatePacketBursty(packetCount++, currentTime, burst);
                }

                if (inputQueues[i].size() < BUFFER_SIZE) {
                    inputQueues[i].push(pkt);
                    cout << "Packet " << pkt.id << " generated at input port " << i << " destined for output port " << pkt.outputPort << endl;
                } else {
                    droppedPackets++;
                    cout << "Buffer overflow at input port " << i << " - Packet dropped!" << endl;
                }
            
        }

        // Attempt to schedule packets based on the selected algorithm
        vector<Packet> scheduledPackets;

        switch (schedulingChoice) {
            case 1:
                // Use the priority scheduling helper function
                scheduledPackets = helperPriority(inputQueues);
                break;
            case 2:
                // Use the weighted fair queuing helper function
                scheduledPackets = helperWFQ(inputQueues, wfqWeights);
                break;
            case 3:
                // Use the round robin helper function
                scheduledPackets = helperRoundRobin(inputQueues, lastScheduledPort);
                break;
            case 4:
                // Use the iSLIP helper function
                scheduledPackets = helperiSLIP(inputQueues, iSLIPPointers);
                break;
            default:
                cout << "Invalid scheduling algorithm selection!" << endl;
                return 1;
        }

        // Process the scheduled packets
        for (const auto& pkt : scheduledPackets) {
                int waitingTime = currentTime - pkt.arrivalTime;
                int turnaroundTime = waitingTime + pkt.processingTime;

                totalWaitingTime += waitingTime;
                totalTurnaroundTime += turnaroundTime;
                processedPackets++;

                // Buffer occupancy calculation
                int bufferOccupancy = 0;
                for (int j = 0; j < NUM_PORTS; ++j) {
                    bufferOccupancy += inputQueues[j].size();
                }

                // Simulate sending the scheduled packet to the output queue
                // outputQueues[pkt.outputPort].push(pkt);
                cout << "Packet " << pkt.id << " scheduled to output port " << pkt.outputPort
                     << " | Waiting Time: " << waitingTime
                     << " | Turnaround Time: " << turnaroundTime
                     << " | Buffer Occupancy: " << bufferOccupancy << endl;
        }

        currentTime++;
    }

    for(int i =0;i<NUM_PORTS;i++){
        droppedPackets += inputQueues[i].size();
    }

    // Output the final statistics after simulation is complete
    cout << "\nSimulation completed!" << endl;
    cout << "Total Packets Processed: " << processedPackets << endl;
    cout << "Total Packets Dropped: " << droppedPackets << endl;
    cout << "Total Packets Generated: " << totalGeneratedPackets << endl;
    cout << "Packet Drop Rate: " << (double)droppedPackets / totalGeneratedPackets * 100 << "%\n";
    cout << "Average Waiting Time: " << (processedPackets > 0 ? (double)totalWaitingTime / processedPackets : 0) << endl;
    cout << "Average Turnaround Time: " << (processedPackets > 0 ? (double)totalTurnaroundTime / processedPackets : 0) << endl;
    cout << "Queue Throughput: " << (double)processedPackets / currentTime << " packets per unit time" << endl;

    return 0;
}