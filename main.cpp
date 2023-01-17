#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <fstream>

#define LOG() std::cout << std::setw(10) << takt << std::setw(10) << process.processId << std::setw(10) << process.name << std::setw(10) << pc << std::setw(10) << acc << std::setw(10) << command << std::endl;
#define LOG_P() std::cout << std::setw(10) << takt << std::setw(10) << process.processId << std::setw(10) << process.name << std::setw(10) << pc  << std::setw(10) << acc << std::setw(10) << "--" << std::endl;

static uint pid = 0;
const uint waitTime = 4;
uint runningSimulation = 1;
uint quantumCounter = 0;
uint quantum = 4;

enum class status {
    running = 0, ready = 1, blocked = 2
};
struct Seitentabellen{

};
struct MMU {
    std::vector<std::byte> RAM;
};
struct Process {
    uint start;
    uint end;
    uint processId;
    std::string name;
    std::vector<std::string> VectorInstruction;
    uint index;
    uint data;
    uint tmp_acc;
    status processStatus;
    uint wait;

    Process() {

        processId = pid;
        pid++;
        data = 0;
        index = 0;
        processStatus = status::ready;
        wait = 0;
    }
};

struct Scheduler {
    std::vector<Process> readyProcesses;
    std::vector<Process> waitingProcesses;
    std::vector<Process> doneProcesses;

    bool getReadyProcess() {
        if (readyProcesses.size() > 0) return true;
        return false;
    }

    void updateWait() {
        if (waitingProcesses.size() > 0) {
            for (unsigned int i = 0; i < waitingProcesses.size(); i++) {
                waitingProcesses.at(i).wait--;
                if (waitingProcesses.at(i).wait == 0) {
                    waitingProcesses.at(i).processStatus = status::ready;
                    if(!getReadyProcess()) waitingProcesses.at(i).processStatus = status::running;
                    readyProcesses.push_back(waitingProcesses.at(i));
                    waitingProcesses.erase(waitingProcesses.begin() + i);
                }
            }
        }
        else {
            return;
        }
    }

    void blockProcess(Process process) {
        if (process.processStatus == status::blocked) return;
        process.processStatus = status::blocked;
        process.wait = waitTime;
        waitingProcesses.push_back(process);
        for (uint i = 0; i < readyProcesses.size(); i++) {
            if (process.processId == readyProcesses.at(i).processId)
                readyProcesses.erase(readyProcesses.begin() + i);
        }
    }

    Process& getRunningProcess() {
        for (Process& p : readyProcesses) {
            if (p.processStatus == status::running) return p;
        }
    }

    void switchProcess() {
        readyProcesses.at(0).processStatus = status::running;
    }

    void destroyProcess() {
        for (unsigned int i = 0; i < readyProcesses.size(); i++) {
            if (readyProcesses.at(i).processStatus == status::running) {
                readyProcesses.erase(readyProcesses.begin() + i);
            }
        }
        if (getReadyProcess()) switchProcess();
        else if (waitingProcesses.size() == 0) runningSimulation = 0;
    }

};

struct Cpu {
    uint takt = 0;
    uint pc = 0;
    uint acc = 0;


    void init(Scheduler& scheduler) {
        createProcess(scheduler, "init");
        execute(scheduler, scheduler.readyProcesses.front());
    }

    void createProcess(Scheduler& scheduler, std::string program) {
        std::string path =  program;
        std::string opcode, variable;
        std::ifstream infile(path);
        Process newProcess;

        newProcess.name = program;
        newProcess.index = 0;//scheduler.doneProcesses.push_back(newProcess);
        newProcess.processStatus = status::running;
        newProcess.start = takt+1;





        //Read File
        while (infile >> opcode) {
            if (opcode != "P" && opcode != "Z") {
                infile >> variable; newProcess.VectorInstruction.push_back(opcode + " " + variable);
            }
            else {
                variable = " ";
                newProcess.VectorInstruction.push_back(opcode + variable);
            }

        }
        scheduler.readyProcesses.push_back(newProcess);
    }

    void execute(Scheduler& scheduler, Process process) {
        while (runningSimulation == 1) {
            scheduler.updateWait();
            if (scheduler.getReadyProcess()) {
                quantumCounter = 0;
                process = scheduler.getRunningProcess();
                pc = process.index;
                acc = process.data;
                takt++;
            }
            else {
                takt++;
                LOG_P();
                continue;
            }

            while (pc < process.VectorInstruction.size()) {
                quantumCounter++;
                std::string command = process.VectorInstruction.at(pc);
                LOG();

                std::string opcode, variable;
                std::string delimiter = " ";

                size_t pos = command.find(delimiter);
                opcode = command.substr(0, command.find(delimiter));
                variable = command.substr(pos + 1);
                if(quantumCounter%quantum == 0 ){
                    opcode = "P";

                    pc-=1;
                }
                //LOAD
                if (opcode.at(0) == 'L') {
                    acc = std::stoi(variable);
                }
                //ADD
                else if (opcode.at(0) == 'A') {
                    acc += std::stoi(variable);
                }
                //SUB
                else if (opcode.at(0) == 'S') {
                    acc -= std::stoi(variable);
                }
                //Peripheral
                else if (opcode.at(0) == 'P') {
                    process.index = pc + 1;
                    process.data = acc;
                    scheduler.blockProcess(process);
                    if (scheduler.getReadyProcess()) {
                        scheduler.switchProcess();
                        break;
                    }
                    else break;

                }
                //Execute
                else if (opcode.at(0) == 'X') {

                    scheduler.getRunningProcess().index = pc + 1;
                    scheduler.getRunningProcess().data = acc;
                    scheduler.getRunningProcess().processStatus = status::ready;

                createProcess(scheduler, variable);
                break;
                }
                //End Programm
                else if (opcode.at(0) == 'Z') {
                    process.tmp_acc = acc;
                    scheduler.destroyProcess();
                    process.end = takt;
                    scheduler.doneProcesses.push_back(process);
                    break;
                }
                pc++;
                takt++;
                scheduler.updateWait();
            }
        }
    }
};

int main() {
    std::cout << "Quantum = " << quantumCounter << std::endl;
    std::cout << std::setw(10) << "Takt " << std::setw(10) << "PID " << std::setw(10) << "Process " << std::setw(10) << "PC " << std::setw(10) << "Acc " << std::setw(10) << "Befehl" << std::endl;
    Cpu cpu;
    Scheduler scheduler;
    cpu.init(scheduler);

    std::cout << "Zusammenfassung der Simulation: " << std::endl;

    std::cout << '\n';

    std::cout << "Simulierte Takte: " << cpu.takt << std::endl;

    for (uint i = 0; i < scheduler.doneProcesses.size(); i++) {
         std::cout << scheduler.doneProcesses[i].name << std::endl;
         std::cout << "Start: " << scheduler.doneProcesses[i].start << std::endl;
         std::cout << "End: " << scheduler.doneProcesses[i].end << std::endl;
         std::cout << "VerweilZeit: " << scheduler.doneProcesses[i].end - scheduler.doneProcesses[i].start +1 << std::endl;
         std::cout << "ACC: " << scheduler.doneProcesses[i].tmp_acc << std::endl;
         std::cout << '\n';
    }

    std::cout << "Quantum = " << quantumCounter << std::endl;

    std::cout << "Simulation end" << std::endl;
    return 0;
}
