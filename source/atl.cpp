#include <iostream>
#include "aig.h"
#include "atl.h"
using namespace std;


StateFormula::StateFormula (oper o, StateFormula * l, StateFormula * r) 
{
  op = o;
  data.left = l;
  data.right = r;
}

StateFormula::StateFormula (string p) {
  op = Atomic;
  data.prop = new string(p);
}
  

StateFormula StateFormula::operator~ () {
  return StateFormula(Not,this,NULL);
}

StateFormula StateFormula::operator| (StateFormula f) {
  StateFormula * g = new StateFormula(f);
  return StateFormula(Or,this,g);
}

StateFormula StateFormula::operator& (StateFormula f) {
  StateFormula * g = new StateFormula(f);
  return StateFormula(And,this,g);
}


BDD StateFormula::of_aiger(BDDAIG & aig, const aiger & spec) {
  if(op == Atomic){
    for(unsigned i=0; i<spec.num_latches; i++) 
      if(*data.prop == spec.latches[i].name)
	return aig.ofLit(spec.latches[i].lit);
    for(unsigned i=0; i<spec.num_outputs; i++) 
      if(*data.prop == spec.outputs[i].name)
	return aig.ofLit(spec.outputs[i].lit);
    for(unsigned i=0; i<spec.num_inputs; i++)
      if(*data.prop == spec.inputs[i].name)
	return aig.ofLit(spec.inputs[i].lit);
    cerr << "Error: no variable named " << *data.prop << " in the given circuit." << endl;
    return aig.manager()->bddZero();
  } else {
    BDD l = data.left->of_aiger(aig,spec);
    if(op == Not)
      return (~l);
    else {
      BDD r = data.right->of_aiger(aig,spec);
       if (op == Or) return (l | r);
       else {
	 assert(op == And);
	 return (l & r);
       }
    }
  }
}
  
void StateFormula::display(ostream & out) {
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

ostream & operator<<(ostream & out, StateFormula sf) 
{
  sf.display(out);
  return out;
}



ATLFormula::ATLFormula(modality m, string s, ATLFormula * l, ATLFormula * r) {
  mod = m;
  coalition = new string(s);
  left = l;
  right = r;
}

ATLFormula::ATLFormula(StateFormula s) : mod(M_SF) 
{
  sf = new StateFormula(s);
}

ATLFormula ATLFormula::AU(string c, ATLFormula r) {
  ATLFormula * l = new ATLFormula(*this);
  ATLFormula * rf = new ATLFormula(r);
  return ATLFormula(M_AU,c,l,rf);    
}

ATLFormula ATLFormula::AX(string c) {
  ATLFormula * l = new ATLFormula(*this);
  return ATLFormula(M_AX,c,l,NULL);    
}

ATLFormula ATLFormula::EU(string c, ATLFormula r) {
  ATLFormula * l = new ATLFormula(*this);
  ATLFormula * rf = new ATLFormula(r);
  return ATLFormula(M_EU,c,l,rf);    
}

ATLFormula ATLFormula::EX(string c) {
  ATLFormula * l = new ATLFormula(*this);
  return ATLFormula(M_EX,c,l,NULL);    
}

void ATLFormula::display(ostream & out) {
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



ostream & operator<<(ostream & out, ATLFormula a) 
{
  a.display(out);
  return out;
}

int main(int argc, char ** argv)
{
  StateFormula h("push_1<0>");
  StateFormula w("accept<0>");
  StateFormula x = h | w;
  cout << x << endl;

  Cudd mgr(0, 0);
  mgr.AutodynEnable(CUDD_REORDER_SIFT);
  if(argc < 2)
    printf("usage: %s aiger_file.aig\n", argv[0]);
  else {
    AIG aig(argv[1]);
    BDDAIG bdd_aig(aig,&mgr);
    BDD b = x.of_aiger(bdd_aig,*aig.spec);
    cout << "Writing test.dot" << endl;
    bdd_aig.dump2dot(b,"test.dot");
  }
  
  ATLFormula f = ATLFormula(x).AX("a1");
  cout << f << endl;
}
