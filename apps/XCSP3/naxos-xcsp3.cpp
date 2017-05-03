/// @file
/// Links the XCSP3 parser to Naxos solver
///
/// Part of https://github.com/pothitos/naxos

#include "translator.h"
#include <XCSP3CoreParser.h>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace naxos;
using namespace std;

bool searching;
bool interrupted;
Xcsp3_to_Naxos *solver;

void printSolutionAndExit(void)
{
        if (solver->solutionIsRecored()) {
                // A solution has been found
                // Status SATISFIABLE or OPTIMUM FOUND has been already printed
                solver->printSolution();
        } else {
                // No solution found and search hasn't been completed
                cout << "s UNKNOWN\n";
        }
        exit(0);
}

void interruptionHandler(int /*signum*/)
{
        if (searching) {
                // The interruption happened inside pm.nextSolution()
                if (solver->solutionIsRecored())
                        cout << "s SATISFIABLE\n";
                printSolutionAndExit();
        } else {
                // Flag interruption, but continue execution
                interrupted = true;
        }
}

int main(int argc, char* argv[])
{
        try {
                // Check if the verbose argument '-v' exists
                bool verbose = false;
                if (argc == 3 && string(argv[2]) == "-v") {
                        verbose = true;
                } else if (argc != 2) {
                        cerr << "Usage: " << argv[0] << " BENCHNAME\n";
                        return 1;
                }
                // Interface between the parser and the solver
                Xcsp3_to_Naxos callbacks(verbose);
                solver = &callbacks;
                XCSP3Core::XCSP3CoreParser parser(solver);
                parser.parse(argv[1]);
                NsIntVar* vObjectivePointer = 0;
                // vObjectivePointer = &Var[2];
                // pm.minimize(*vObjectivePointer);
                // Initialize variables for search
                interrupted = false;
                searching = true;
                // Register interruption signal handler
                signal(SIGINT, interruptionHandler);
                while (solver->nextSolution() != false) {
                        searching = false;
                        solver->recordSolution();
                        if (vObjectivePointer == 0) {
                                cout << "s SATISFIABLE\n";
                                printSolutionAndExit();
                                // Non-optimization search finishes here
                        } else {
                                cout << "o " << vObjectivePointer->value()
                                     << endl;
                        }
                        searching = true;
                        // Check if interrupted while in critical area
                        if (interrupted)
                                interruptionHandler(SIGINT); // resume function
                }
                // Search has been completed
                searching = false;
                if (solver->solutionIsRecored()) {
                        cout << "s OPTIMUM FOUND\n";
                        printSolutionAndExit();
                } else {
                        cout << "s UNSATISFIABLE\n";
                }
        } catch (exception& exc) {
                cerr << exc.what() << "\n";
                return 1;
        } catch (...) {
                cerr << "Unknown exception\n";
                return 1;
        }
}
