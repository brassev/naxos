/// @file
/// Links the XCSP3 parser to Naxos solver
///
/// Part of https://github.com/pothitos/naxos

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <naxos.h>

using namespace naxos;
using namespace std;

bool in_critical_area;
bool interrupted;
NsIntVarArray* VarSolution;

void printSolutionAndExit(void)
{
        if (VarSolution == 0) {
                // No solution found and search hasn't been completed
                cout << "s UNKNOWN\n";
        } else {
                // A solution has been found
                // Status SATISFIABLE or OPTIMUM FOUND has been already printed
                cout << "v <instantiation>\n"
                     << "v   <list> x[] </list>\n"
                     << "v   <values> " << *VarSolution << " </values>\n"
                     << "v </instantiation>\n";
        }
        exit(0);
}

void interruptionHandler(int /*signum*/)
{
        if (in_critical_area)
                interrupted = true; // Flag interruption, but continue execution
        else
                printSolutionAndExit();
}

int main(int argc, char* argv[])
{
        try {
                // Parse the input filename command line argument
                if (argc != 2) {
                        cerr << "Usage: " << argv[0] << " BENCHNAME\n";
                        return 1;
                }
                // State the Constraint Satisfaction Problem
                int N = 8;
                NsProblemManager pm;
                NsIntVarArray Var, VarPlus, VarMinus;
                for (int i = 0; i < N; ++i) {
                        Var.push_back(NsIntVar(pm, 0, N - 1));
                        VarPlus.push_back(Var[i] + i);
                        VarMinus.push_back(Var[i] - i);
                }
                pm.add(NsAllDiff(Var));
                pm.add(NsAllDiff(VarPlus));
                pm.add(NsAllDiff(VarMinus));
                NsIntVar* vObjectivePointer = 0;
                pm.addGoal(new NsgLabeling(Var));
                cout << "c Created " << pm.numVars() << " variables and "
                     << pm.numConstraints()
                     << " constraints, including intermediates\n";
                VarSolution = 0;
                in_critical_area = false;
                interrupted = false;
                // Register interruption signal handler
                signal(SIGINT, interruptionHandler);
                while (pm.nextSolution() != false) {
                        in_critical_area = true;
                        // Record solution
                        VarSolution = &Var;
                        if (vObjectivePointer == 0) {
                                signal(SIGINT, SIG_IGN); // No more interruption
                                cout << "s SATISFIABLE\n";
                                printSolutionAndExit();
                        } else {
                                cout << "o " << vObjectivePointer->value()
                                     << endl;
                        }
                        in_critical_area = false;
                        // Check if interrupted while in critical area
                        if (interrupted) {
                                cout << "s SATISFIABLE\n";
                                printSolutionAndExit();
                        }
                }
                // Search has been completed
                signal(SIGINT, SIG_IGN); // No more interruption
                if (vObjectivePointer == 0) {
                        // Non-optimization search didn't find a solution
                        cout << "s UNSATISFIABLE\n";
                } else {
                        cout << "s OPTIMUM FOUND\n";
                        printSolutionAndExit();
                }
        } catch (exception& exc) {
                cerr << exc.what() << "\n";
                return 1;
        } catch (...) {
                cerr << "Unknown exception\n";
                return 1;
        }
}
