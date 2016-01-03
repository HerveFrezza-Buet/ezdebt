#ifndef ezdebtASSURANCE_H
#define ezdebtASSURANCE_H

namespace ezdebt {
  class Assurance {
  public:
    double cout;        // 2.5...
    double montant_ref; // ... pour 10000 empruntes.
    double couverture;  // ex : 120 pour 120%
    
    Assurance(void) 
      : cout(0), 
	montant_ref(100),
	couverture(100) {}
    ~Assurance(void) {}
    Assurance(const Assurance& cp)
      : cout(cp.cout),
	montant_ref(cp.montant_ref),
	couverture(cp.couverture) {}

    Assurance& operator=(const Assurance& cp) {
      cout        = cp.cout;
      montant_ref = cp.montant_ref;
      couverture  = cp.couverture;
      return *this;
    }
  };
}

#endif
