#ifndef ezdebtPLACEMENT_H
#define ezdebtPLACEMENT_H

#include <cmath>
#include "ezdebtEvenement.h"

namespace ezdebt {
  
  class Placement : public Produit {
  public:

    double taux;
    double capital;
    double total_interets;
    double total_virements;
    double virement;
    double dernier_virement; // trace du dernier événement de virement.
    double virement_ponctuel;

    Placement(void) 
      : Produit(),
	taux(0),
	capital(0),
	total_interets(0),
	total_virements(0),
	virement(0),
	dernier_virement(0),
	virement_ponctuel(0) {}
    virtual ~Placement(void) {}
    Placement(const Placement& cp) 
      : Produit(cp),
	taux(cp.taux),
	capital(cp.capital),
	total_interets(cp.total_interets),
	total_virements(cp.total_virements),
	virement(cp.virement),
	dernier_virement(cp.dernier_virement),
	virement_ponctuel(cp.virement_ponctuel) {}

    Placement& operator=(const Placement& cp) {
      Produit::operator=(cp);
      taux                = cp.taux;
      capital             = cp.capital;
      total_interets      = cp.total_interets;
      total_virements     = cp.total_virements;
      virement            = cp.virement;
      dernier_virement    = cp.dernier_virement;
      virement_ponctuel   = cp.virement_ponctuel;
      return *this;
    }

    virtual double tauxReel(void) const {
      return pow((1+taux/100),1/12.0)-1;
    }

    virtual void step(void) {
      double gain;

      virement = virement_ponctuel;
      virement_ponctuel = 0;
      total_virements += virement;
      capital += virement;

      gain = capital*tauxReel();
      capital += gain;
      total_interets += gain;

      updateFrais();
    }



    class FraisPlace : public Evenement {
    public:
      double taux;
      FraisPlace(std::string nom_placement,
		 double taux_frais) 
	: Evenement(nom_placement),
	  taux(taux_frais){}
      virtual ~FraisPlace(void) {}
      virtual void execute(Produit& p) {
	Placement* placement = (Placement*)(&p);
	double vrt = fabs(placement->dernier_virement);

	placement->declareFrais(vrt*taux/100.0);
      }
      virtual std::string description(void) {
	std::ostringstream ostr;
	
	ostr << "Frais de " << taux << "% sur le dernier virement du placement \"" << produit
	     << "\".";
	
	return ostr.str();
      }
    };

    class Place : public Evenement {
    public:
      double montant;
      Place(std::string nom_placement,
	    double montant_placement) 
	: Evenement(nom_placement),
	  montant(montant_placement){}
      virtual ~Place(void) {}
       virtual void execute(Produit& p) {
	Placement* placement = (Placement*)(&p);
	placement->en_cours = true;
	placement->dernier_virement = montant;
	placement->virement_ponctuel += montant;
      }
      virtual std::string description(void) {
	std::ostringstream ostr;
	
	if(montant>=0)
	  ostr << "Virement de " << montant;
	else
	  ostr << "Retrait de " << -montant;

	ostr << " sur le placement \"" << produit
	     << "\".";
	
	return ostr.str();
      }
    };

    
  };
}


#endif
