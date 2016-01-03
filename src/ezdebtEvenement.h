#ifndef ezdebtEVENEMENT_H
#define ezdebtEVENEMENT_H

#include <string>

namespace ezdebt {

  class Produit;

  class Evenement {
  public:
    
    std::string produit;

    Evenement(std::string nom_produit) 
      : produit(nom_produit) {}
    virtual ~Evenement(void) {}
    virtual void execute(Produit& p)=0;
    virtual std::string description(void)=0;
  };


  class Produit {
  public:
    bool en_cours;
    double taux;
    double total_frais;
    double frais;    
    double frais_tmp;

    Produit(void) 
      : en_cours(false),
	taux(0),
	total_frais(0),
	frais(0),  
	frais_tmp(0) {} 
    virtual ~Produit(void) {}

    Produit(const Produit& cp)
      : en_cours(cp.en_cours),
	taux(cp.taux),
	total_frais(cp.total_frais),
	frais(cp.frais),
	frais_tmp(cp.frais_tmp) {}
    
    Produit& operator=(const Produit& cp) {
      en_cours           = cp.en_cours;
      taux               = cp.taux;
      total_frais        = cp.total_frais;
      frais              = cp.frais;
      frais_tmp          = cp.frais_tmp;
      return *this;
    }

    virtual void clore(void) {
	en_cours   = false;
    }

    virtual double tauxReel(void) const = 0;

    double fraisDuMois(void) const {
      return frais;
    }

    void declareFrais(double f) {
      frais_tmp += f;
    }

    void updateFrais(void) {
      frais = frais_tmp;
      total_frais += frais;
      frais_tmp=0;
    }

    virtual void step(void) {}

    class FraisFixe : public Evenement {
    public:
      double montant;
      
      FraisFixe(std::string nom_produit,
	   double montant_frais) 
	: Evenement(nom_produit),
	  montant(montant_frais) {}
      virtual ~FraisFixe(void) {}
      virtual void execute(Produit& p) {
	p.declareFrais(montant);
      }

      virtual std::string description(void) {
	std::ostringstream ostr;
	
	ostr << "Frais fixe sur le produit \"" << produit 
	     << "\" de " << montant << ".";
	
	return ostr.str();
      }
    };


    class Cloture : public Evenement {
    public:
      
      Cloture(std::string nom_produit) 
	: Evenement(nom_produit) {}
      virtual ~Cloture(void) {}
      virtual void execute(Produit& p) {
	p.clore();
      }

      virtual std::string description(void) {
	std::ostringstream ostr;
	
	ostr << "Clôture du produit \"" << produit 
	     << "\"";
	
	return ostr.str();
      }
    };

    class Taux : public Evenement {
    public:
      double taux;
      
      Taux(std::string nom_produit,
	   double taux_produit) 
	: Evenement(nom_produit),
	  taux(taux_produit) {}
      virtual ~Taux(void) {}
       virtual void execute(Produit& p) {
	p.taux = taux;
      }

      virtual std::string description(void) {
	std::ostringstream ostr;
	
	ostr << "Changement du taux du produit \"" 
	     << produit
	     << "\" à " << taux << "%.";
	
	return ostr.str();
      }
    };

  };

}

#endif
