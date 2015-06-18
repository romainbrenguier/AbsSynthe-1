#include "aig.h"

#ifndef ATL_H
#define ATL_H

class StateFormula {
 public:
  typedef enum { Atomic, Or, And, Not } oper;
  
  StateFormula(std::string);
  StateFormula operator~ ();
  StateFormula operator| (StateFormula f);
  StateFormula operator& (StateFormula f);
  BDD of_aiger(BDDAIG & aig, const aiger & spec);
  void display(std::ostream & out);
  
 private:
  typedef union {
    std::string * prop;
    struct {
      StateFormula * left; 
      StateFormula * right;
    };
  } state_formula_data;


  oper op;
  state_formula_data data;

  StateFormula (oper o, StateFormula * l, StateFormula * r);
};

std::ostream & operator<<(std::ostream & out, StateFormula sf);


class ATLFormula {
public:
  typedef enum { M_AU, M_AX, M_EU, M_EX, M_SF} modality;
  ATLFormula(StateFormula s);
  ATLFormula AU(std::string c, ATLFormula r);
  ATLFormula AX(std::string c);
  ATLFormula EU(std::string c, ATLFormula r);
  ATLFormula EX(std::string c);
  void display(std::ostream & out);

private:
  modality mod;
  std::string * coalition;
  ATLFormula * left;
  ATLFormula * right;
  StateFormula * sf;

  ATLFormula(modality m, std::string s, ATLFormula * l, ATLFormula * r);
};

std::ostream & operator<<(std::ostream & out, ATLFormula a);


#endif
