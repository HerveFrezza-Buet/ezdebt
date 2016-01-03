#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cstdlib>

#include <string>
#include <map>
#include <list>

#include "ezdebtPlacement.h"
#include "ezdebtPret.h"
#include "ezdebtEvenement.h"
#include "ezdebtAssurance.h"

std::map<std::string,ezdebt::Pret> prets;
std::map<std::string,ezdebt::Placement> placements;
std::map<int,std::list<ezdebt::Evenement*> > echeancier;

#define ezdebtPAGESIZE 12


void parse(std::istream& is);
void eatComment(std::istream& is);
void registerEcheancier(int date,ezdebt::Evenement* evt);
void simulate(std::string prefix);
void step(void);
bool toutInactif(void);
void echeance(std::map<int,std::list<ezdebt::Evenement*> >::iterator& iter,int time);
std::string prix(double p);
std::string prix(bool display,double p);
bool freeName(std::string tag,std::string name);
bool freePretName(std::string tag,std::string name);
bool freePlacementName(std::string tag,std::string name);
bool existsPret(std::string tag,std::string name);
bool existsPlacement(std::string tag,std::string name);
bool existsProduit(std::string tag,std::string name);


void htmlHeader(std::string prefix,
		std::ostream& file);
void htmlFooter(std::ostream& file);
void htmlListePlacements(std::ostream& file);
void htmlHistorique(std::ostream& file);
void htmlBeginTime(std::ostream& file);
void htmlEndTime(std::ostream& file);
void htmlBeginTimeBlock(std::ostream& file);
void htmlEndTimeBlock(std::ostream& file);
void htmlTimeLine(std::ostream& file, int date);
void htmlBilan(std::ostream& file);

int main(int argc, char* argv[]) {
  if(argc!=2) {
    std::cout << "Usage: " << argv[0] << " file.ezd" << std::endl;
    return 1;
  }

  std::ifstream ifs;
  std::string filename = argv[1];
  ifs.open(filename.c_str());
  if(!ifs) {
    std::cerr << "Cannot open \"" << filename << "\". aborting." << std::endl;
    return 1;
  }

  std::cout << std::endl << std::endl
	    << "Lecture du fichier \"" << filename << "\"..."
	    << std::endl << std::endl;
  parse(ifs);
  std::cout << std::endl
	    << "... lecture terminée."
	    << std::endl << std::endl;
  ifs.close();

  int pos=0;
  for(unsigned int i=0;i<filename.length();++i)
    if(filename[i]=='/')
      pos = (int)i;
  if((pos==0) && (filename[0]!='/'))
    pos = -1;
  std::string prefix = filename.substr(pos+1,filename.length()-pos-5);

  simulate(prefix);

  return 0;
}

void eatComment(std::istream& is) {
  char c;
  std::string line;
  is >> c;
  while(c=='#' && !(is.eof())) {
    std::getline(is,line);
    is >> c;
  }
  if(!(is.eof()))
    is.putback(c);
}

std::string prix(bool display,double p) {
  if(display)
    return prix(p);
  else
    return "";
}


std::string format_prix(double p) {
  std::ostringstream ostr;

  if(p<0) {
    p = -p;
    ostr << "-";
  }
    

  ostr << std::fixed << std::setprecision(2)
       << .01*((int)(100*p+.5));

  return ostr.str();
}

std::string prix(double p) {

  if(fabs(p)<.005)
    return "";

  return format_prix(p);
}

void report(ezdebt::Evenement* evt,
	    int date) {
  std::cout << "Mois " << std::setw(4) << date << " : " << evt->description() << std::endl;
}

void parse(std::istream& is) {
  std::string command;
  std::string name,buf;
  int date,duree;
  double montant,taux;
  ezdebt::Pret pret;
  ezdebt::Placement placement;
  ezdebt::Assurance assurance;

  eatComment(is);
  while(!(is.eof())) {
    is >> command;

    if(command == "Pret") {
      eatComment(is); is >> name;
      if(freeName("Pret",name)) {
	eatComment(is); is >> pret.montant;
	eatComment(is); is >> pret.taux;
	prets[name] = pret;
	std::cout << "Déclaration du prêt \"" << name 
		  << "\" : " << pret.montant << " à " 
		  << pret.taux << "%." << std::endl;
      }
    }

    else if(command == "Placement") {
      eatComment(is); is >> name;
      if(freeName("Placement",name)) {
	eatComment(is); is >> placement.taux;
	placements[name] = placement;
	std::cout << "Déclaration du placement \"" << name 
		  << "\" : à " 
		  << placement.taux << "%." << std::endl;
      }
    }

    else if(command == "Assure") {
      eatComment(is); is >> name;
      if(existsPret("Assure",name)) {
	eatComment(is); is >> assurance.cout;
	eatComment(is); is >> buf; // pour
	eatComment(is); is >> assurance.montant_ref;
	eatComment(is); is >> buf; // a
	eatComment(is); is >> assurance.couverture;
	eatComment(is); is >> buf; // au
	eatComment(is); is >> buf; // mois
	eatComment(is); is >> date; 
	
	ezdebt::Pret::Assure* evt_assure = new ezdebt::Pret::Assure(name,assurance);
	registerEcheancier(date,evt_assure);
	report(evt_assure,date);
      }
    }

    else if(command == "Contracte") {
      eatComment(is); is >> name;
      if(existsPret("Contracte",name)) {
	eatComment(is); is >> buf; // au
	eatComment(is); is >> buf; // mois
	eatComment(is); is >> date; 
	ezdebt::Pret::Contracte* evt_contracte = new ezdebt::Pret::Contracte(name);
	registerEcheancier(date,evt_contracte);
	report(evt_contracte,date);
      }
    }

    else if(command == "FraisDette") {
      eatComment(is); is >> name;
      if(existsPret("FraisDette",name)) {
	eatComment(is); is >> taux;
	eatComment(is); is >> buf; // au
	eatComment(is); is >> buf; // mois
	eatComment(is); is >> date; 
	ezdebt::Pret::FraisDette* evt_fraisdette = new ezdebt::Pret::FraisDette(name,taux);
	registerEcheancier(date,evt_fraisdette);
	report(evt_fraisdette,date);
      }
    }

    else if(command == "FraisFixe") {
      eatComment(is); is >> name;
      if(existsProduit("FraisFixe",name)) {
	eatComment(is); is >> montant;
	eatComment(is); is >> buf; // au
	eatComment(is); is >> buf; // mois
	eatComment(is); is >> date; 
	ezdebt::Produit::FraisFixe* evt_fraisfixe = new ezdebt::Produit::FraisFixe(name,montant);
	registerEcheancier(date,evt_fraisfixe);
	report(evt_fraisfixe,date);
      }
    }

    else if(command == "Taux") {
      eatComment(is); is >> name;
      if(existsProduit("Taux",name)) {
	eatComment(is); is >> taux;
	eatComment(is); is >> buf; // au
	eatComment(is); is >> buf; // mois
	eatComment(is); is >> date; 
	ezdebt::Produit::Taux* evt_taux = new ezdebt::Produit::Taux(name,taux);
	registerEcheancier(date,evt_taux);
	report(evt_taux,date);
      }
    }

    else if(command == "Rembourse") {
      eatComment(is); is >> name;
      if(existsPret("Rembourse",name)) {
	eatComment(is); is >> buf; // sur
	eatComment(is); is >> duree; 
	eatComment(is); is >> buf; // mois
	eatComment(is); is >> buf; // depuis
	eatComment(is); is >> date;  
	ezdebt::Pret::Rembourse* evt_rembourse = new ezdebt::Pret::Rembourse(name,duree);
	registerEcheancier(date,evt_rembourse);
	report(evt_rembourse,date);
      }
    }

    else if(command == "Traite") {
      eatComment(is); is >> name;
      if(existsPret("Traite",name)) {
	eatComment(is); is >> buf; // de
	eatComment(is); is >> montant; 
	eatComment(is); is >> buf; // depuis
	eatComment(is); is >> date; 
	ezdebt::Pret::Traite* evt_traite = new ezdebt::Pret::Traite(name,montant);
	registerEcheancier(date,evt_traite);
	report(evt_traite,date);
      }
    }

    else if(command == "Stabilise") {
      eatComment(is); is >> name;
      if(existsPret("Stabilise",name)) {
	eatComment(is); is >> buf; // au
	eatComment(is); is >> buf; // mois
	eatComment(is); is >> date; 
	ezdebt::Pret::Stabilise* evt_stabilise = new ezdebt::Pret::Stabilise(name);
	registerEcheancier(date,evt_stabilise);
	report(evt_stabilise,date);
      }
    }

    else if(command == "Solde") {
      eatComment(is); is >> name;
      if(existsPret("Solde",name)) {
	eatComment(is); is >> buf; // au
	eatComment(is); is >> buf; // mois
	eatComment(is); is >> date; 
	ezdebt::Pret::Solde* evt_solde = new ezdebt::Pret::Solde(name);
	registerEcheancier(date,evt_solde);
	report(evt_solde,date);
      }
    }

    else if(command == "Anticipe") {
      eatComment(is); is >> name;
      if(existsPret("Anticipe",name)) {
	eatComment(is); is >> buf; // de
	eatComment(is); is >> montant; 
	eatComment(is); is >> buf; // au
	eatComment(is); is >> buf; // mois
	eatComment(is); is >> date; 
	ezdebt::Pret::Anticipe* evt_anticipe = new ezdebt::Pret::Anticipe(name,montant);
	registerEcheancier(date,evt_anticipe);
	report(evt_anticipe,date);
      }
    }

    else if(command == "Cloture") {
      eatComment(is); is >> name;
      if(existsProduit("Cloture",name)) {
	eatComment(is); is >> buf; // au
	eatComment(is); is >> buf; // mois
	eatComment(is); is >> date; 
	ezdebt::Produit::Cloture* evt_cloture = new ezdebt::Produit::Cloture(name);
	registerEcheancier(date,evt_cloture);
	report(evt_cloture,date);
      }
    }


    else if(command == "Place") {
      eatComment(is); is >> name;
      if(existsPlacement("Place",name)) {
	eatComment(is); is >> montant; 
	eatComment(is); is >> buf; // au
	eatComment(is); is >> buf; // mois
	eatComment(is); is >> date; 
	ezdebt::Placement::Place* evt_place = new ezdebt::Placement::Place(name,montant);
	registerEcheancier(date,evt_place);
	report(evt_place,date);
      }
    }

    else if(command == "FraisPlace") {
      eatComment(is); is >> name;
      if(existsPlacement("FraisPlace",name)) {
	eatComment(is); is >> taux; 
	eatComment(is); is >> buf; // au
	eatComment(is); is >> buf; // mois
	eatComment(is); is >> date; 
	ezdebt::Placement::FraisPlace* evt_frais_place = new ezdebt::Placement::FraisPlace(name,taux);
	registerEcheancier(date,evt_frais_place);
	report(evt_frais_place,date);
      }
    }

    else
      std::cerr << "###### Command \"" << command 
		<< "\" is not valid. Ignored. ######" 
		<< std::endl;

    eatComment(is);
  }
}

void registerEcheancier(int date,ezdebt::Evenement* evt) {
  echeancier[date].push_back(evt);
}


void simulate(std::string prefix) {
  
  std::ofstream html;
  std::string html_name;
  int time;
  std::map<int,std::list<ezdebt::Evenement*> >::iterator ech_iter;
  std::map<std::string,ezdebt::Pret>::iterator iter;
  double just_closed=false;

  ech_iter = echeancier.begin();
  if(ech_iter == echeancier.end()) {
    std::cout << "Il n'y a rien à simuler..." << std::endl;
    return;
  }

  html_name = prefix + ".html";
  html.open(html_name.c_str());

  htmlHeader(prefix,html);
  
  htmlListePlacements(html);
  htmlHistorique(html);

  htmlBeginTime(html);
  
  for(time=1;ech_iter!=echeancier.end() || !toutInactif(); ++time) {
    if(ech_iter!=echeancier.end())
      echeance(ech_iter,time);
    step();
    just_closed=false;
    if(!toutInactif()) {
      if((time-1)%ezdebtPAGESIZE == 0) {
	if(time > 1) {
	  htmlEndTimeBlock(html);
	  just_closed=true;
	}
	htmlBeginTimeBlock(html);
      }
      htmlTimeLine(html,time);
    }
  }
  if(!just_closed)
    htmlEndTimeBlock(html);
  htmlEndTime(html);

  htmlBilan(html);
  htmlFooter(html);
  html.close();
}

void step(void) {
  std::map<std::string,ezdebt::Pret>::iterator iter1;
  std::map<std::string,ezdebt::Placement>::iterator iter2;
  for(iter1 = prets.begin(); iter1 != prets.end(); ++iter1)
    iter1->second.step();
  for(iter2 = placements.begin(); iter2 != placements.end(); ++iter2)
    iter2->second.step();
}


bool toutInactif(void) {
  std::map<std::string,ezdebt::Pret>::iterator iter1;
  std::map<std::string,ezdebt::Placement>::iterator iter2;
  bool res = true;
  
  

  if(prets.empty()) {
    // S'il n'y a pas de prêts, on regarde les placements.
    for(iter2 = placements.begin(); res && iter2 !=placements.end(); ++iter2)
      res = ! (iter2->second.en_cours);
  }
  else {
    // S'il y a des prêts, leur fin génère l'arrêt de la simulation.
    for(iter1 = prets.begin(); res && iter1 !=prets.end(); ++iter1)
      res = ! (iter1->second.en_cours);
  }

  return res;
}

void echeance(std::map< int,std::list<ezdebt::Evenement*> >::iterator& iter,int time) {
  std::list<ezdebt::Evenement*>::iterator liter;
  ezdebt::Evenement* e;

  if((*iter).first==time) {
    for(liter=(*iter).second.begin();liter!=(*iter).second.end();++liter) {
      e = *liter;
      if(prets.count(e->produit)!=0)
	e->execute(prets[e->produit]);
      else if(placements.count(e->produit)!=0)
	e->execute(placements[e->produit]);
      else
	std::cerr << "Bad product name \"" << e->produit << "\" in echeance, please report this bug."
		  << std::endl;
    }
    ++iter;
  }
}


bool freePretName(std::string tag,std::string name) {
  bool res = false;
  if(prets.count(name)!=0)
    std::cerr << tag << " : Il y a déjà un prêt nommé \"" << name <<"\"." << std::endl;
  else
    res = true;
  return res;
}

bool freePlacementName(std::string tag,std::string name) {
  bool res = false;
  if(placements.count(name)!=0)
    std::cerr << tag << " : Il y a déjà un placement nommé \"" << name <<"\"." << std::endl;
  else
    res = true;
  return res;
}

bool freeName(std::string tag,std::string name) {
  return freePretName(tag,name) && freePlacementName(tag,name);
}

bool existsPret(std::string tag,std::string name) {
  bool res = false;
  if(prets.count(name)==0) 
    std::cerr << tag << " : Le prêt \"" << name << "\" n'existe pas." << std::endl;
  else
    res = true;
  return res;
}

bool existsPlacement(std::string tag,std::string name) {
  bool res = false;
  if(placements.count(name)==0) 
    std::cerr << tag << " : Le placement \"" << name << "\" n'existe pas." << std::endl;
  else
    res = true;
  return res;
}

bool existsProduit(std::string tag,std::string name) {
  bool res = false;
  if((prets.count(name)==0) && (placements.count(name)==0))
    std::cerr << tag << " : Le produit \"" << name << "\" n'existe pas." << std::endl;
  else
    res = true;
  return res;
}

void htmlHeader(std::string prefix,
		std::ostream& file) {
  file << "<html>" << std::endl
       << "<head>" << std::endl
       << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>" << std::endl
       << "<title>" << prefix << "</title>" << std::endl
       << "<link href=\""
       << ezdebtDATA_DIR
       << "/ezdebt.css\" type=\"text/css\" rel=\"stylesheet\"/>" << std::endl
       << "</head>" << std::endl
       << "<body>" << std::endl
       << std::endl;
}

void htmlFooter(std::ostream& file) {
  file << std::endl
       << "<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>"
       << "</body>" << std::endl
       << "</html>" << std::endl;
}

void htmlListePlacements(std::ostream& file) {
  std::map<std::string,ezdebt::Pret>::iterator iter1;
  std::map<std::string,ezdebt::Placement>::iterator iter2;
  file << std::endl
       << "<div id=\"section\">Produits banquaires</div>"
       << std::endl
       << "<div id=\"titlepdt\">Prêts</div>" << std::endl
       << std::endl
       << "<div id=\"listeprets\">" << std::endl
       << "  <table id=\"prets\">" << std::endl;
  for(iter1 = prets.begin(); iter1 != prets.end(); ++iter1)
    file << "    <tr> <td id=\"nompret\">" << iter1->first << "</th> "
	 << "<td id=\"montantpret\">" << prix(iter1->second.montant) << "</td> "
	 << "<td id=\"tauxpret\">" << format_prix(iter1->second.taux) << "%</td> "
	 << "</tr>" << std::endl;
  file << "  </table>" << std::endl
       << "</div>" << std::endl
       << std::endl
       << "<div id=\"titlepdt\">Placements</div>" << std::endl
       << std::endl
       << "<div id=\"listeplacements\">" << std::endl
       << "  <table id=\"placements\">" << std::endl;
  for(iter2 = placements.begin(); iter2 != placements.end(); ++iter2)
    file << "    <tr> <td id=\"nomplacement\">" << iter2->first << "</th> "
	 << "<td id=\"tauxplacement\">" << prix(iter2->second.taux) << "%</td> "
	 << "</tr>" << std::endl;
  file << "  </table>" << std::endl
       << "</div>" << std::endl
       << std::endl;
}

void htmlHistorique(std::ostream& file) {
  std::map<int,std::list<ezdebt::Evenement*> >::iterator ech_iter;
  std::list<ezdebt::Evenement*>::iterator liter;

  file << std::endl
       << "<div id=\"section\">Historique</div>"
       << std::endl
       << "<div id=\"historique\">" << std::endl;

  for(ech_iter = echeancier.begin();
      ech_iter != echeancier.end();
      ++ech_iter) {
    file << std::endl
	 << "  <div id=\"moishistorique\">" << std::endl
	 << "    <div id=\"historiquetitre\">Mois " << ech_iter->first << "</div>" << std::endl
	 << "    <ul id=\"historiqueitem\">" << std::endl;
    for(liter=(*ech_iter).second.begin();
	liter!=(*ech_iter).second.end();
	++liter) 
      file << "      <li>" << (*liter)->description() << std::endl;
    
    file << "    </ul>" << std::endl
	 << "  </div>" << std::endl;
  }
  
  file << std::endl
       << "</div>"
       << std::endl;
}

void htmlBeginTime(std::ostream& file) {
  file << std::endl
       << "<div id=\"section\">Echéancier</div>"
       << std::endl
       << "<div id=\"time\">" << std::endl;
}

void htmlEndTime(std::ostream& file) {
  file << std::endl
       << "</div>"
       << std::endl;
}

void htmlBeginTimeBlock(std::ostream& file) {
  std::map<std::string,ezdebt::Pret>::iterator iter1;
  std::map<std::string,ezdebt::Placement>::iterator iter2;

  file << std::endl
       << "  <table id=\"timeblock\">" << std::endl;

  file << "    <tr>" << std::endl;
  for(iter1 = prets.begin(); iter1 != prets.end(); ++iter1) 
    file << "        <th id=\"timetitlebar\" colspan=\"6\">" 
	 << iter1->first << "</th>"<< std::endl;
  if(!prets.empty())
    file << "        <th id=\"timetitlebar\"/>" << std::endl;
  for(iter2 = placements.begin(); iter2 != placements.end(); ++iter2) 
    file << "        <th id=\"timetitlebar\" colspan=\"4\">" 
	 << iter2->first << "</th>"<< std::endl;
  file << "    </tr>" << std::endl; 
  
  file << "    <tr>" << std::endl; 
  for(iter1 = prets.begin(); iter1 != prets.end(); ++iter1) {
    file << "        <th id=\"timetitlebar\">Mois</th>" << std::endl
	 << "        <th id=\"timetitlebar\">Restant dû</th>" << std::endl
	 << "        <th id=\"timetitlebar\">Remboursement</th>" << std::endl
	 << "        <th id=\"timetitlebar\">Intérêts</th>" << std::endl
	 << "        <th id=\"timetitlebar\">Frais</th>" << std::endl
	 << "        <th id=\"timetitlebar\">Total</th>" << std::endl;
  }
  if(!prets.empty())
    file << "        <th id=\"timetitlebar\">Total prêts</th>" << std::endl;

  for(iter2 = placements.begin(); iter2 != placements.end(); ++iter2) 
    file << "        <th id=\"timetitlebar\">Mois</th>" << std::endl
	 << "        <th id=\"timetitlebar\">Capital</th>" << std::endl
	 << "        <th id=\"timetitlebar\">Virement</th>" << std::endl
	 << "        <th id=\"timetitlebar\">Frais</th>" << std::endl;
  file << "    </tr>" << std::endl;
}

void htmlEndTimeBlock(std::ostream& file) {
  file << "  </table>" << std::endl;
}

void htmlTimeLine(std::ostream& file, int date) {
  std::map<std::string,ezdebt::Pret>::iterator iter1;
  std::map<std::string,ezdebt::Placement>::iterator iter2;
  std::string suffix;
  double total_banque,total;

  if(date%2 == 0)
    suffix="pair";
  else
    suffix="impair";
  total_banque = 0;
  file << "    <tr>" << std::endl;
  for(iter1 = prets.begin(); iter1 != prets.end(); ++iter1) {
    file << "        <td id=\"timepretmois" << suffix << "\">" << date << "</td>" << std::endl
	 << "        <td id=\"timepretdette" << suffix << "\">" << prix(iter1->second.en_cours,
								    iter1->second.dette) << "</td>" << std::endl
	 << "        <td id=\"timepretversement" << suffix << "\">" << prix(iter1->second.en_cours,
									iter1->second.versement) << "</td>" << std::endl
	 << "        <td id=\"timepretinteret" << suffix << "\">" << prix(iter1->second.en_cours,
									iter1->second.part_interets) << "</td>" << std::endl
	 << "        <td id=\"timepretfrais" << suffix << "\">" << prix(iter1->second.en_cours,
								    iter1->second.frais) << "</td>" << std::endl;
    total = iter1->second.frais+iter1->second.versement;
    file << "        <td id=\"timeprettotal" << suffix << "\">" << prix(iter1->second.en_cours,
								    total) << "</td>" << std::endl;
    if(iter1->second.en_cours)
      total_banque += total;
  }
  if(!prets.empty())
    file << "        <td id=\"timeprettotal" << suffix << "\">" << prix(total_banque) << "</td>";

  for(iter2 = placements.begin(); iter2 != placements.end(); ++iter2) 
    file << "        <td id=\"timeplacementmois" << suffix << "\">" << date << "</td>"
	 << "        <td id=\"timeplacementcapital" << suffix << "\">" << prix(iter2->second.en_cours,
									       iter2->second.capital) << "</td>" << std::endl
	 << "        <td id=\"timeplacementvirement" << suffix << "\">" << prix(iter2->second.en_cours,
										iter2->second.virement) << "</td>" << std::endl
	 << "        <td id=\"timeplacementfrais" << suffix << "\">" << prix(iter2->second.en_cours,
									     iter2->second.frais) << "</td>" << std::endl;

  file <<"    </tr>" << std::endl;
}

void htmlBilan(std::ostream& file) {
  std::map<std::string,ezdebt::Pret>::iterator iter1;
  std::map<std::string,ezdebt::Placement>::iterator iter2;
  double total_montant;
  double total_interet;
  double total_frais;
  double interet=0;
  double cout;
  double gain;
  double bilan=0;
  file << std::endl
       << "<div id=\"section\">Bilan</div>" << std::endl
       << std::endl
       << "<div id=\"bilan\">" << std::endl;

  if(!prets.empty()) {

    total_montant=0;
    total_interet=0;
    total_frais=0;

    file << "  <table id=\"bilanblock\">" << std::endl
	 << "    <tr><th id=\"bilantitre\">Prêt</th>" << std::endl
	 << "        <th id=\"bilantitre\">Montant</th>" << std::endl
	 << "        <th id=\"bilantitre\">Intêrets</th>" << std::endl
	 << "        <th id=\"bilantitre\">Frais</th>" << std::endl
	 << "        <th id=\"bilantitre\">Coût</th>" << std::endl
	 << "        <th id=\"bilantitre\">Coût relatif</th>" << std::endl
	 << "    </tr>" << std::endl;
    for(iter1 = prets.begin(); iter1 != prets.end(); ++iter1) {
      total_montant   += iter1->second.montant;
      total_frais     += iter1->second.total_frais;
      interet          = iter1->second.total_versement - iter1->second.montant;
      total_interet   += interet;
      cout             = interet + iter1->second.total_frais;
      file << "    <tr><td id=\"bilanpretnom\">" << iter1->first << "</td>" << std::endl
	   << "        <td id=\"bilanpretmontant\">" << prix(iter1->second.montant) << "</td>" << std::endl
	   << "        <td id=\"bilanpretinterets\">" << prix(interet) << "</td>" << std::endl
	   << "        <td id=\"bilanpretfrais\">" << prix(iter1->second.total_frais) << "</td>" << std::endl
	   << "        <td id=\"bilanpretcouttotal\">" << prix(cout) << "</td>" << std::endl
	   << "        <td id=\"bilanpretcoutrelatif\">" << prix(100*cout/(iter1->second.montant)) << "%</td>" << std::endl
	   << "    </tr>" << std::endl;
    }
    
    cout = total_interet + total_frais;
    file<< "    <tr><td id=\"bilanprettotalnom\">Total</td>" << std::endl
	<< "        <td id=\"bilanprettotalmontant\">" << prix(total_montant) << "</td>" << std::endl
	<< "        <td id=\"bilanprettotalinterets\">" << prix(total_interet) << "</td>" << std::endl
	<< "        <td id=\"bilanprettotalfrais\">" << prix(total_frais) << "</td>" << std::endl
	<< "        <td id=\"bilanprettotalcouttotal\">" << prix(cout) << "</td>" << std::endl
	<< "        <td id=\"bilanprettotalcoutrelatif\">" << prix(100*cout/total_montant) << "%</td>" << std::endl
	<< "    </tr>" << std::endl;
    
    file << "  </table>" << std::endl;

    bilan -= cout;
  }


  if(!placements.empty()) {

    total_montant=0;
    total_interet=0;
    total_frais=0;

    file << "  <table id=\"bilanblock\">" << std::endl
	 << "    <tr><th id=\"bilantitre\">Placement</th>" << std::endl
	 << "        <th id=\"bilantitre\">Virements</th>" << std::endl
	 << "        <th id=\"bilantitre\">Intêrets</th>" << std::endl
	 << "        <th id=\"bilantitre\">Frais</th>" << std::endl
	 << "        <th id=\"bilantitre\">Gain</th>" << std::endl
	 << "    </tr>" << std::endl;
    for(iter2 = placements.begin(); iter2 != placements.end(); ++iter2) {
      total_montant   += iter2->second.total_virements;
      total_frais     += iter2->second.total_frais;
      total_interet   += iter2->second.total_interets;
      gain             = iter2->second.total_interets - iter2->second.total_frais;
      file << "    <tr><td id=\"bilanplacementnom\">" << iter2->first << "</td>" << std::endl
	   << "        <td id=\"bilanplacementvirements\">" << prix(iter2->second.total_virements) << "</td>" << std::endl
	   << "        <td id=\"bilanplacementinterets\">" << prix(iter2->second.total_interets) << "</td>" << std::endl
	   << "        <td id=\"bilanplacementfrais\">" << prix(iter2->second.total_frais) << "</td>" << std::endl
	   << "        <td id=\"bilanplacementgaintotal\">" << prix(gain) << "</td>" << std::endl
	   << "    </tr>" << std::endl;
    }
    
    gain = total_interet - total_frais;
    file<< "    <tr><td id=\"bilanplacementtotalnom\">Total</td>" << std::endl
	<< "        <td id=\"bilanplacementtotalvirements\">" << prix(total_montant) << "</td>" << std::endl
	<< "        <td id=\"bilanplacementtotalinterets\">" << prix(total_interet) << "</td>" << std::endl
	<< "        <td id=\"bilanplacementtotalfrais\">" << prix(total_frais) << "</td>" << std::endl
	<< "        <td id=\"bilanplacementtotalgaintotal\">" << prix(gain) << "</td>" << std::endl
	<< "    </tr>" << std::endl;
    
    file << "  </table>" << std::endl;

    bilan +=gain;
  }
  
  if(bilan>=0)
    file << "<div id=\"bilanpositif\">Gain de <b>" << prix(bilan) << "</b></div>" << std::endl;
  else
    file << "<div id=\"bilannegatif\">Perte de <b>" << prix(-bilan) << "</b></div>" << std::endl;

  file << "</div>" << std::endl;
}

