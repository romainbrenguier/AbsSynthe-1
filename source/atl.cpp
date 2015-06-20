#include <iostream>
#include "aig.h"
#include "atl.h"
using namespace std;
typedef unsigned lit;

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


lit find_lit(const aiger & spec, string name)
{
  for(unsigned i=0; i<spec.num_latches; i++) 
    if(name == spec.latches[i].name)
      return spec.latches[i].lit;
  for(unsigned i=0; i<spec.num_outputs; i++) 
    if(name == spec.outputs[i].name)
      return spec.outputs[i].lit;
  for(unsigned i=0; i<spec.num_inputs; i++)
    if(name == spec.inputs[i].name)
      return spec.inputs[i].lit;
  cerr << "Error: no variable named " << name << " in the given circuit." << endl;
  return -1;
}

BDD StateFormula::to_bdd(BDDAIG & aig, const aiger & spec) {
  if(op == Atomic){
    return aig.ofLit(find_lit(spec,*data.prop));
    //return aig.manager()->bddZero();
  } else {
    BDD l = data.left->to_bdd(aig,spec);
    if(op == Not)
      return (~l);
    else {
      BDD r = data.right->to_bdd(aig,spec);
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

ATLFormula ATLFormula::operator&(ATLFormula r) {
  ATLFormula * l = new ATLFormula(*this);
  ATLFormula * rf = new ATLFormula(r);
  return ATLFormula(M_AND,"",l,rf);    
}

ATLFormula ATLFormula::operator|(ATLFormula r) {
  ATLFormula * l = new ATLFormula(*this);
  ATLFormula * rf = new ATLFormula(r);
  return ATLFormula(M_OR,"",l,rf);    
}

ATLFormula ATLFormula::operator~() {
  ATLFormula * l = new ATLFormula(*this);
  return ATLFormula(M_NOT,"",l,NULL);    
}

/*
ATLFormula::~ATLFormula(){
  if(mod == M_SF) delete sf;
  else {
    delete left;
    if (mod == M_AND || mod == M_OR || mod == M_AU || mod == M_EU)
      delete right;
    if (mod <= M_EX)
      delete coalition;
  }
}
*/
void ATLFormula::display(ostream & out) {
  out << "(";
  switch(mod){
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
  case M_AND: 
    out << *left << " & " << *right;
    break;
  case M_OR: 
    out << *left << " | " << *right;
    break;
  case M_NOT: 
    out << "~ " << *left;
    break;
  }
  out << ")";
}

vector<lit> input_of_coalition(const aiger & spec, string name)
{
  vector<lit> v;
  for(unsigned i=0; i<spec.num_inputs; i++)
    if(name == spec.inputs[i].name)
      v.push_back(spec.inputs[i].lit);

  return v;
}


vector<lit> input_of_opposite_coalition(const aiger & spec, string name)
{
  vector<lit> v;
  for(unsigned i=0; i<spec.num_inputs; i++)
    if(name != spec.inputs[i].name)
      v.push_back(spec.inputs[i].lit);

  return v;
}

pair<BDD,BDD> cpre(BDDAIG & aig, BDD dst, BDD cinput_cube, BDD uinput_cube)
{
  BDD not_dst = ~dst;
  vector<BDD> next_funs = aig.nextFunComposeVec(&not_dst);
  BDD trans_bdd = dst.VectorCompose(next_funs);
  BDD temp_bdd = trans_bdd.UnivAbstract(cinput_cube);
  pair<BDD,BDD> p(temp_bdd.ExistAbstract(uinput_cube), trans_bdd);
  return p;
}

BDD cube(BDDAIG & aig, vector<lit> lits)
{
  BDD c = aig.manager()->bddOne();
  for(unsigned i = 0; i < lits.size(); i++)
    c &= aig.ofLit(lits[i]);
  return c;
}

BDD ATLFormula::to_bdd(BDDAIG & aig, const aiger & spec)
{
  if(mod == M_AU || mod == M_EU) {
    BDD goal = right->to_bdd(aig,spec);
    BDD constraint = left->to_bdd(aig,spec);
    vector<lit> uncontrollable;
    vector<lit> controllable;
    if(mod == M_EU)
      {
	uncontrollable = input_of_coalition(spec,*coalition);
	controllable = input_of_opposite_coalition(spec,*coalition);
      }
    else
      {
	controllable = input_of_coalition(spec,*coalition);
	uncontrollable = input_of_opposite_coalition(spec,*coalition);
      }
    BDD uncontr_cube = cube(aig,uncontrollable);
    BDD contr_cube = cube(aig,controllable);
    bool fixpoint = false;
    while(! fixpoint) {
      pair<BDD,BDD> c = cpre(aig,goal,contr_cube,uncontr_cube);
      BDD f = constraint & c.first;
      if(goal == f)
	fixpoint = true;
      else
	goal = f;
    }
    return goal;
  } 
  
  else if(mod == M_AX || mod == M_EX) {
    BDD goal = left->to_bdd(aig,spec);
    vector<lit> uncontrollable;
    vector<lit> controllable; 
    if(mod == M_AX) {
      uncontrollable = input_of_coalition(spec,*coalition);
      controllable = input_of_opposite_coalition(spec,*coalition);
    } else {
      assert(mod == M_EX);
      controllable = input_of_coalition(spec,*coalition);
      uncontrollable = input_of_opposite_coalition(spec,*coalition);
    }
    BDD uncontr_cube = cube(aig,uncontrollable);
    BDD contr_cube = cube(aig,controllable);
    pair<BDD,BDD> f = cpre(aig,goal,contr_cube,uncontr_cube);
    return f.first;
  }

  else if(mod == M_SF) 
    return sf->to_bdd(aig,spec);
  else if(mod == M_AND)
    return (left->to_bdd(aig,spec) & right->to_bdd(aig,spec));
  else if(mod == M_OR)
    return (left->to_bdd(aig,spec) | right->to_bdd(aig,spec));
  else {
    assert (mod == M_NOT);
    return (~ left->to_bdd(aig,spec));
  }
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
  StateFormula y = h & w;
  cout << x << endl;

  ATLFormula f = ATLFormula(x).AX("controllable_fill_1<0>") | ATLFormula(y).EX("controllable_empty_1<0>") ;
  cout << f << endl;

  Cudd mgr(0, 0);
  mgr.AutodynEnable(CUDD_REORDER_SIFT);
  if(argc < 2)
    printf("usage: %s aiger_file.aig\n", argv[0]);
  else {
    AIG aig(argv[1]);
    BDDAIG bdd_aig(aig,&mgr);
    BDD b = x.to_bdd(bdd_aig,*aig.spec);
    cout << "Writing test.dot" << endl;
    bdd_aig.dump2dot(b,"test.dot");


    BDD winning = f.to_bdd(bdd_aig,*aig.spec);
    cout << "Writing winning.dot" << endl;
    bdd_aig.dump2dot(winning,"winning.dot");
  }
  

}
