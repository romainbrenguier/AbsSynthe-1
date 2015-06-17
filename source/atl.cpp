#include <iostream>
//#include <regex>
#include "aig.h"
using namespace std;


class StateFormula {
public:
  typedef enum { Atomic, Or, And, Not } oper;

private:
  typedef union {
    string * prop;
    struct {
      StateFormula * left; 
      StateFormula * right;
    };
  } state_formula_data;


  oper op;
  state_formula_data data;

  StateFormula (oper o, StateFormula * l, StateFormula * r) 
  {
    op = o;
    data.left = l;
    data.right = r;
  }

public:
  StateFormula (string p) {
    op = Atomic;
    data.prop = new string(p);
  }
  

  StateFormula operator~ () {
    return StateFormula(Not,this,NULL);
  }

  StateFormula operator| (StateFormula f) {
    StateFormula * g = new StateFormula(f);
    return StateFormula(Or,this,g);
  }

  StateFormula operator& (StateFormula f) {
    StateFormula * g = new StateFormula(f);
    return StateFormula(And,this,g);
  }

  BDD of_aiger(BDDAIG aig, aiger spec) {
    switch(op) {
    case Atomic:
      for(unsigned i=0; i<spec.num_latches; i++) {
	cout << spec.latches[i].name << endl; 
	if(*data.prop == spec.latches[i].name)
	  return aig.ofLit(spec.latches[i].lit);
      }
      for(unsigned i=0; i<spec.num_outputs; i++) {
	cout << spec.outputs[i].name << endl; 
	if(*data.prop == spec.outputs[i].name)
	  return aig.ofLit(spec.outputs[i].lit);
      }
      for(unsigned i=0; i<spec.num_inputs; i++) {
	cout << spec.inputs[i].name << endl; 
	if(*data.prop == spec.inputs[i].name)
	  return aig.ofLit(spec.inputs[i].lit);
      }
      break;
    }
  }
  
  void display(ostream & out) {
    out << "(";
    switch(op) {
    case Atomic : 
      out << *data.prop;
      break;
    case Or :
      data.left->display(out);
      out << " or ";
      data.right->display(out);
      break;
    case And :
      data.left->display(out);
      out << " and ";
      data.right->display(out);
      break;
    case Not :
      out << " not ";
      data.left->display(out);
      break;
    }
    out << ")";
  }
};

ostream & operator<<(ostream & out, StateFormula sf) 
{
  sf.display(out);
  return out;
}

class ATLFormula {
public:
  typedef enum { M_AU, M_AX, M_EU, M_EX, M_SF} modality;

private:
  modality mod;
  string * coalition;
  ATLFormula * left;
  ATLFormula * right;
  StateFormula * sf;

  ATLFormula(modality m, string s, ATLFormula * l, ATLFormula * r) {
    mod = m;
    coalition = new string(s);
    left = l;
    right = r;
  }

public: 
  ATLFormula(StateFormula s) : mod(M_SF) 
  {
    sf = new StateFormula(s);
  }

  ATLFormula AU(string c, ATLFormula r) {
    ATLFormula * l = new ATLFormula(*this);
    ATLFormula * rf = new ATLFormula(r);
    return ATLFormula(M_AU,c,l,rf);    
  }

  ATLFormula AX(string c) {
    ATLFormula * l = new ATLFormula(*this);
    return ATLFormula(M_AX,c,l,NULL);    
  }

  ATLFormula EU(string c, ATLFormula r) {
    ATLFormula * l = new ATLFormula(*this);
    ATLFormula * rf = new ATLFormula(r);
    return ATLFormula(M_EU,c,l,rf);    
  }

  ATLFormula EX(string c) {
    ATLFormula * l = new ATLFormula(*this);
    return ATLFormula(M_EX,c,l,NULL);    
  }

  void display(ostream & out) {
    out << "(";
    switch(mod) {
    case M_AU: 
      out << "[" << *coalition << "] ";
      left->display(out);
      out << " U ";
      right->display(out);
      break;
    case M_AX: 
      out << "[" << *coalition << "] X ";
      left->display(out);
      break;
    case M_EU: 
      out << "<" << *coalition << "> ";
      left->display(out);
      out << " U ";
      right->display(out);
      break;
    case M_EX: 
      out << "<" << *coalition << "> X ";
      left->display(out);
      break;
    case M_SF:
      out << *sf;
      break;
    }
    out << ")";
  }

};

ostream & operator<<(ostream & out, ATLFormula a) 
{
  a.display(out);
  return out;
}

int main()
{
  StateFormula h("hello");
  StateFormula w("world");
  StateFormula x = h | w;
  cout << x << endl;
  ATLFormula f = ATLFormula(x).AX("a1");
  cout << f << endl;
}
