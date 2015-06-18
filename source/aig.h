/**************************************************************************
 * Copyright (c) 2015, Guillermo A. Perez, Universite Libre de Bruxelles
 * 
 * This file is part of the (Swiss) AbsSynthe tool.
 * 
 * AbsSynthe is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * AbsSynthe is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with AbsSynthe.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * 
 * Guillermo A. Perez
 * Universite Libre de Bruxelles
 * gperezme@ulb.ac.be
 *************************************************************************/

#ifndef ABSSYNTHE_AIG_H
#define ABSSYNTHE_AIG_H

#include <string>
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <cassert>

#include "cuddObj.hh"

#include "aiger.h"

/* A class for AIG (And Inverted Graphs) */
class AIG {
 public:
  // Initialize an AIG by reading it from a file.
  // When the parameter intro_error_latch is set to true, then we will add an additional latch for the error output.
  AIG(const char*, bool intro_error_latch=true);

        // Initialize an AIG by copying an existing one.
        AIG(const AIG&);

        ~AIG();

	// Take a literal as input and give the literal representing its negation.
	static unsigned negateLit(unsigned lit) { return lit ^ 1; }

	// Tells if a literal represent the negation of a gate or input.
	static bool litIsNegated(unsigned lit) { return (lit & 1) == 1; }
	// Return a non-negated version of the literal.
        static unsigned stripLit(unsigned lit) { return lit & ~1; }

	// Maximum variable that is used in the AIG.
        // (the literal that represents a variable is obtained by multiplying it by two).
        unsigned maxVar();

	// Add a gate the current AIG. 
        // The first argument is the literal corresponding to the gate to be added, and the two following arguments are the literals of which this gate should compute the conjunction.
        void addGate(unsigned, unsigned, unsigned);

        // The first argument is a literal representing an input of the circuit and the second a gate. ????
        void input2gate(unsigned, unsigned);

        void writeToFile(const char*);
        std::vector<aiger_symbol*> getLatches() { return this->latches; }
        std::vector<aiger_symbol*> getCInputs() { return this->c_inputs; }
        unsigned numLatches();
        std::vector<unsigned> getCInputLits();
        std::vector<unsigned> getUInputLits();
        std::vector<unsigned> getLatchLits();
///Why is this public?
        void removeErrorLatch();


	// I'm making this public because it can be usefull
        aiger* spec;

    private:
        bool must_clean;
        void cleanCaches();

    protected:
        std::vector<aiger_symbol*> latches;
        std::vector<aiger_symbol*> c_inputs;
        std::vector<aiger_symbol*> u_inputs;
        aiger_symbol* error_fake_latch;


	
        void introduceErrorLatch();
        std::unordered_map<unsigned, std::set<unsigned>>* lit2deps_map;
        std::unordered_map<unsigned,
                           std::pair<std::vector<unsigned>,
                                     std::vector<unsigned>>>* lit2ninputand_map;
        void getNInputAnd(unsigned, std::vector<unsigned>*,
                          std::vector<unsigned>*);
        void getLitDepsRecur(unsigned, std::set<unsigned>&,
                             std::unordered_set<unsigned>*);
        std::set<unsigned> getLitDeps(unsigned);
        static unsigned primeVar(unsigned lit) { return AIG::stripLit(lit) + 1; }
};

class BDDAIG : public AIG {

    public:
	// Initialization from an AIG and a manager
        BDDAIG(const AIG&, Cudd*);
	// Initialization by copying an existing BDDAIG and ???
        BDDAIG(const BDDAIG&, BDD);
	// Destructor.
        ~BDDAIG();

	// The result coincide with the first argument on the valuation that satisfy the second argument, and is smaller than the first argument.
        static BDD safeRestrict(BDD, BDD);
	// ??
        std::set<unsigned> semanticDeps(BDD);
	// Returns a literal used to represent its next valuation
        static unsigned primeVar(unsigned lit) { return AIG::stripLit(lit) + 1; }
        void dump2dot(BDD, const char*);
	// Returns a BDD representing the initial state.
        BDD initState();
	// Returns a BDD representing the error states.
        BDD errorStates();
	// Returns a BDD where all latches have been replaced by the corresponding next literal.
        BDD primeLatchesInBdd(BDD);
	// Returns a BDD where all primed latches have been replaced by the original (unprimed) literal.
        BDD unprimeLatchesInBdd(const BDD &);
	// Cube containing the latch literals 
        BDD primedLatchCube();
	// Cube containing the controllable inputs literals 
        BDD cinputCube();
	// Cube containing the uncontrollable inputs literals 
        BDD uinputCube();
	// BDD representing the transition relation ?????
        BDD transRelBdd();
	// Takes a set of literals and return the corresponding cube
        BDD toCube(std::set<unsigned>&);
	BDD ofLit(unsigned);
        std::set<unsigned> getBddDeps(BDD);
        std::set<unsigned> getBddLatchDeps(BDD);

	/*
	 * Returns for each latch the BDD that represents its update function.
	 * The argument is represents a care region, it is set to NULL by default. */
        std::vector<BDD> nextFunComposeVec(BDD* care_region=NULL);
        std::vector<BDDAIG*> decompose();

	Cudd* manager();

    private:
        bool must_clean;

    protected:
        Cudd* mgr;
        BDD* primed_latch_cube;
        BDD* cinput_cube;
        BDD* uinput_cube;
        BDD* trans_rel;
        BDD* short_error;
        std::unordered_map<unsigned, BDD>* lit2bdd_map;
        std::unordered_map<unsigned long, std::set<unsigned>>* bdd2deps_map;
        std::vector<BDD>* next_fun_compose_vec;
        BDD lit2bdd(unsigned);
        std::vector<BDD> mergeSomeSignals(BDD, std::vector<unsigned>*);
        bool isValidLatchBdd(BDD);
        bool isValidBdd(BDD);
};

#endif
