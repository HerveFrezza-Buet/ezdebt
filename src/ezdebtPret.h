#ifndef ezdebtPRET_H
#define ezdebtPRET_H

#include <cmath>
#include "ezdebtAssurance.h"
#include "ezdebtEvenement.h"

#define EPSILON .001
namespace ezdebt {
  
  class Pret : public Produit {
  public:

    double montant;
    Assurance assurance;
    double dette;
    double next_dette;
    double cout_assurance;
    double total_versement;
    double versement; // Pour éponger la dette
    double versement_ponctuel;
    double part_interets;
    double versement_mensuel;

    Pret(void) 
      : Produit(),
	montant(0), 
	assurance(),
	dette(0),
	next_dette(0),
	cout_assurance(0),
	total_versement(0),
	versement(0),
	versement_ponctuel(0),  
	part_interets(0),  
	versement_mensuel(0) {}
    virtual ~Pret(void) {}

    Pret(const Pret& cp)
      : Produit(cp),
	montant(cp.montant),
	assurance(cp.assurance),
	dette(cp.dette),
	next_dette(cp.next_dette),
	cout_assurance(cp.cout_assurance),
	total_versement(cp.total_versement),
	versement(cp.versement),
	versement_ponctuel(cp.versement_ponctuel),
	part_interets(cp.part_interets),
	versement_mensuel(cp.versement_mensuel) {}

    Pret& operator=(const Pret& cp) {
      Produit::operator=(cp);
      montant            = cp.montant;
      taux               = cp.taux;
      assurance          = cp.assurance;
      dette              = cp.dette;
      next_dette         = cp.next_dette;
      cout_assurance     = cp.cout_assurance;
      total_versement    = cp.total_versement;
      versement          = cp.versement;
      versement_ponctuel = cp.versement_ponctuel;
      part_interets      = cp.part_interets;
      versement_mensuel  = cp.versement_mensuel;
      return *this;
    }

    virtual void clore(void) {
      Produit::clore();
      dette      = 0;
      next_dette = 0;
    }


    double coutAssuranceMensuel(void) const {
      return (montant/assurance.montant_ref)*assurance.cout*(assurance.couverture/100.0)/12.0;
    }

    virtual double tauxReel(void) const {
      return taux/100.0/12.0;
    }

    virtual void step(void) {
      part_interets = 0;

      if(en_cours) {
	
	dette = next_dette;
	if(dette < EPSILON) {
	  clore();
	  return;
	}
      }

      if(en_cours) {
	cout_assurance += coutAssuranceMensuel();
	
	versement = versement_mensuel + versement_ponctuel;
	versement_ponctuel = 0;
	
	next_dette = dette*(1+tauxReel());
	if(versement > next_dette)
	  versement = next_dette;
	part_interets = dette*tauxReel();
	next_dette -= versement;
	total_versement += versement;
      }

      if(en_cours) 
	declareFrais(coutAssuranceMensuel());

      updateFrais();
    }

    class Contracte : public Evenement {
    public:
      
      Contracte(std::string nom_pret) : Evenement(nom_pret) {}
      virtual ~Contracte(void) {}
      virtual void execute(Produit& p) {
	Pret* pret = (Pret*)(&p);
	p.en_cours   = true;
	pret->next_dette = pret->montant;
      }

      virtual std::string description(void) {
	std::ostringstream ostr;
	
	ostr << "Contraction du prêt \"" << produit << "\"."; 
	return ostr.str();
      }
    };

    class Assure : public Evenement {
    public:
      Assurance assurance;
      
      Assure(std::string nom_pret,
	     const Assurance& assurance_pret) 
	: Evenement(nom_pret),
	  assurance(assurance_pret) {}
      virtual ~Assure(void) {}
       virtual void execute(Produit& p) {
	Pret* pret = (Pret*)(&p);
	pret->assurance = assurance;
      }

      virtual std::string description(void) {
	std::ostringstream ostr;
	
	ostr << "Le prêt \"" << produit 
	     << "\" est assuré à " << assurance.couverture
	     << "% :" 
	     << " coût annuel de " << assurance.cout 
	     << " pour " << assurance.montant_ref
	     << " empruntés.";
	
	return ostr.str();
      }
    };


    class FraisDette : public Evenement {
    public:
      double taux;
      
      FraisDette(std::string nom_pret,
	   double taux_frais) 
	: Evenement(nom_pret),
	  taux(taux_frais) {}
      virtual ~FraisDette(void) {}
       virtual void execute(Produit& p) {
	Pret* pret = (Pret*)(&p);
	p.declareFrais(pret->next_dette*taux*.01);
      }

      virtual std::string description(void) {
	std::ostringstream ostr;
	
	ostr << "Frais sur le capital restant dû du prêt \"" 
	     << produit
	     << "\" à " << taux << "%.";
	
	return ostr.str();
      }
    };

    class Rembourse : public Evenement {
    public:
      double duree;
      
      Rembourse(std::string nom_pret,
		int duree_pret) 
	: Evenement(nom_pret),
	  duree(duree_pret) {}
      virtual ~Rembourse(void) {}
       virtual void execute(Produit& p) {
	Pret* pret = (Pret*)(&p);
	if(fabs(pret->taux) > EPSILON)
	  pret->versement_mensuel = pret->next_dette*pret->tauxReel()/(1-pow(1+pret->tauxReel(),-duree));
	else
	  pret->versement_mensuel = pret->next_dette / duree;
      }

      virtual std::string description(void) {
	std::ostringstream ostr;
	
	ostr << "Remboursement du prêt \"" << produit
	     << "\" sur " << duree << " mois.";
	
	return ostr.str();
      }
    };

    class Traite : public Evenement {
    public:
      double montant;
      
      Traite(std::string nom_pret,
		double montant_traite) 
	: Evenement(nom_pret),
	  montant(montant_traite) {}
      virtual ~Traite(void) {}
       virtual void execute(Produit& p) {
	Pret* pret = (Pret*)(&p);
	pret->versement_mensuel = montant;
      }

      virtual std::string description(void) {
	std::ostringstream ostr;
	
	ostr << "Traite sur \"" << produit
		  << "\" de " << montant << " par mois.";
	
	return ostr.str();
      }
    };

    class Stabilise : public Evenement {
    public:
      Stabilise(std::string nom_pret) 
	: Evenement(nom_pret){}
      virtual ~Stabilise(void) {}
       virtual void execute(Produit& p) {
	Pret* pret = (Pret*)(&p);
	pret->versement_mensuel = pret->next_dette*pret->tauxReel();
      }
      virtual std::string description(void) {
	std::ostringstream ostr;
	
	ostr << "Stabilisation du prêt \"" << produit
	     << "\".";
	
	return ostr.str();
      }
    };

    class Solde : public Evenement {
    public:
      Solde(std::string nom_pret) 
	: Evenement(nom_pret){}
      virtual ~Solde(void) {}
       virtual void execute(Produit& p) {
	Pret* pret = (Pret*)(&p);
	pret->versement_ponctuel += pret->next_dette*(1+pret->tauxReel());
	pret->versement_mensuel = 0;
      }

      virtual std::string description(void) {
	std::ostringstream ostr;
	
	ostr << "Solde du prêt \"" << produit
	     << "\".";
	
	return ostr.str();
      }
    };

    class Anticipe : public Evenement {
    public:
      double montant;
      
      Anticipe(std::string nom_pret,
		double montant_anticipe) 
	: Evenement(nom_pret),
	  montant(montant_anticipe) {}
      virtual ~Anticipe(void) {}
       virtual void execute(Produit& p) {
	Pret* pret = (Pret*)(&p);
	pret->versement_ponctuel += montant;
      }

      virtual std::string description(void) {
	std::ostringstream ostr;
	
	ostr << "Anticipe sur \"" << produit 
	     << "\" de " << montant 
	     << ".";
	
	return ostr.str();
      }
    };
    
  };
}

#endif
