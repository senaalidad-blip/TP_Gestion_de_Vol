// Application console - Agence de voyage (Cotonou)
// Gestion des passagers, vols, réservations, paiements et annulations
#include <iomanip>
#include <iostream>
#include <vector>
#include "Agence.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

// ---------- Affichage ----------
void afficherMenu() {
    cout << "\n==================================================\n"
         << "   AGENCE DE VOYAGE - COTONOU\n"
         << "==================================================\n"
         << "  1. Enregistrer un passager\n"
         << "  2. Ajouter un vol\n"
         << "  3. Afficher les vols disponibles\n"
         << "  4. Rechercher un vol\n"
         << "  5. Effectuer une reservation\n"
         << "  6. Enregistrer un paiement\n"
         << "  7. Annuler une reservation\n"
         << "  8. Consulter les reservations\n"
         << "  9. Afficher les informations d'une reservation\n"
         << " 10. Quitter\n"
         << "--------------------------------------------------\n";
}

void afficherTableVols(const vector<const Vol*>& vols) {
    cout << "\n" << left << setw(8) << "Vol" << setw(16) << "Origine" << setw(16) << "Destination"
         << setw(18) << "Depart" << setw(10) << "Places" << "Prix\n"
         << string(84, '-') << "\n";
    for (const Vol* v : vols) {
        ostringstream places;
        places << v->getPlacesDisponibles() << "/" << v->getCapacite();
        cout << left << setw(8) << v->getNumero() << setw(16) << v->getOrigine()
             << setw(16) << v->getDestination() << setw(18) << v->getDateDepart()
             << setw(10) << places.str() << utils::formaterMontant(v->getPrix()) << "\n";
    }
}

vector<const Vol*> tousLesVols(const Agence& a) {
    vector<const Vol*> res;
    for (const auto& v : a.getVols()) res.push_back(v.get());
    return res;
}

// ---------- Actions du menu ----------
void actionEnregistrerPassager(Agence& a) {
    cout << "\n--- Enregistrement d'un passager ---\n";
    string nom = utils::lireNom("Nom : ");
    string prenom = utils::lireNom("Prenom : ");
    string tel = utils::lireTelephone("Telephone : ");
    string piece = utils::majuscules(utils::lireLigne("N° de piece d'identite (passeport/CIP) : "));
    Passager& p = a.enregistrerPassager(nom, prenom, tel, piece);
    cout << "\n[OK] Passager enregistre. Identifiant : " << p.getId() << "\n";
}

void actionAjouterVol(Agence& a) {
    cout << "\n--- Ajout d'un vol ---\n";
    string numero = utils::lireCode("Numero du vol (ex : AF512) : ", 3, 8);
    if (a.trouverVol(numero)) throw logic_error("Le vol " + numero + " existe deja.");
    string origine = utils::lireNom("Ville de depart : ");
    string destination = utils::lireNom("Ville d'arrivee : ");
    string date = utils::lireDateHeure("Date et heure de depart (JJ/MM/AAAA HH:MM) : ");
    int capacite = utils::lireEntier("Capacite (1-600) : ", 1, 600);
    double prix = utils::lireMontant("Prix du billet (FCFA) : ");
    a.ajouterVol(numero, origine, destination, date, capacite, prix);
    cout << "\n[OK] Vol " << numero << " ajoute.\n";
}

void actionAfficherVolsDisponibles(const Agence& a) {
    cout << "\n--- Vols disponibles ---";
    auto vols = a.volsDisponibles();
    if (vols.empty()) { cout << "\nAucun vol disponible.\n"; return; }
    afficherTableVols(vols);
}

void actionRechercherVol(const Agence& a) {
    cout << "\n--- Recherche d'un vol ---\n";
    string cle = utils::lireLigne("Mot-cle (numero, depart ou destination) : ");
    auto vols = a.rechercherVols(cle);
    if (vols.empty()) { cout << "\nAucun vol ne correspond a \"" << cle << "\".\n"; return; }
    cout << "\n" << vols.size() << " vol(s) trouve(s) :";
    afficherTableVols(vols);
}

void actionReserver(Agence& a) {
    cout << "\n--- Nouvelle reservation ---\n";
    if (a.getPassagers().empty()) throw logic_error("Aucun passager enregistre : enregistrez-en un d'abord.");
    if (a.getVols().empty()) throw logic_error("Aucun vol n'est programme.");
    string idP = utils::lireCode("Identifiant du passager (ex : P001) : ", 2, 10);
    if (!a.trouverPassager(idP))
        throw logic_error("Passager introuvable : il doit etre enregistre avant de reserver.");
    afficherTableVols(a.volsDisponibles());
    cout << "\n";
    string numero = utils::lireCode("Numero du vol : ", 3, 8);
    Reservation& r = a.reserver(idP, numero);
    cout << "\n[OK] Reservation " << r.getId() << " creee (EN ATTENTE de paiement).\n"
         << "     Montant a regler : " << utils::formaterMontant(r.getVol()->getPrix()) << "\n";
}

void actionPayer(Agence& a) {
    cout << "\n--- Enregistrement d'un paiement ---\n";
    string id = utils::lireCode("Identifiant de la reservation (ex : R001) : ", 2, 10);
    Reservation* r = a.trouverReservation(id);
    if (!r) throw logic_error("Reservation introuvable : " + id + ".");
    if (r->getStatut() == StatutReservation::ANNULEE)
        throw logic_error("Une reservation annulee ne peut plus etre payee.");
    if (r->getStatut() == StatutReservation::VALIDEE) throw logic_error("Cette reservation est deja payee.");
    cout << "Montant du : " << utils::formaterMontant(r->getVol()->getPrix()) << "\n";
    double montant = utils::lireMontant("Montant verse (FCFA) : ");
    cout << "Mode : 1. Especes  2. Mobile Money  3. Carte bancaire\n";
    int m = utils::lireEntier("Choix du mode (1-3) : ", 1, 3);
    ModePaiement mode = m == 1 ? ModePaiement::ESPECES
                      : m == 2 ? ModePaiement::MOBILE_MONEY : ModePaiement::CARTE;
    Paiement& p = a.enregistrerPaiement(id, montant, mode);
    cout << "\n[OK] Paiement " << p.getId() << " enregistre. Reservation " << id
         << " VALIDEE : la place est definitivement reservee.\n";
}

void actionAnnuler(Agence& a) {
    cout << "\n--- Annulation d'une reservation ---\n";
    string id = utils::lireCode("Identifiant de la reservation : ", 2, 10);
    Reservation* r = a.trouverReservation(id);
    if (!r) throw logic_error("Reservation introuvable : " + id + ".");
    if (r->getStatut() == StatutReservation::ANNULEE)
        throw logic_error("Cette reservation est deja annulee.");
    if (!utils::confirmer("Confirmer l'annulation de " + id)) {
        cout << "Annulation abandonnee.\n";
        return;
    }
    bool immediat = false;
    if (r->getStatut() == StatutReservation::VALIDEE)
        immediat = utils::confirmer("Reservation payee : rembourser immediatement");
    a.annulerReservation(id, immediat);
    cout << "\n[OK] Reservation " << id << " annulee. Remboursement : "
         << versTexte(r->getRemboursement()) << ".\n";
}

void actionConsulterReservations(const Agence& a) {
    cout << "\n--- Liste des reservations ---\n";
    if (a.getReservations().empty()) { cout << "Aucune reservation.\n"; return; }
    cout << left << setw(7) << "ID" << setw(24) << "Passager" << setw(8) << "Vol"
         << setw(18) << "Statut" << "Remboursement\n" << string(76, '-') << "\n";
    for (const auto& r : a.getReservations())
        cout << left << setw(7) << r->getId() << setw(24) << r->getPassager()->nomComplet()
             << setw(8) << r->getVol()->getNumero() << setw(18) << versTexte(r->getStatut())
             << versTexte(r->getRemboursement()) << "\n";
}

void actionInfosReservation(const Agence& a) {
    cout << "\n--- Informations d'une reservation ---\n";
    string id = utils::lireCode("Identifiant de la reservation : ", 2, 10);
    const Reservation* r = a.trouverReservation(id);
    if (!r) throw logic_error("Reservation introuvable : " + id + ".");
    const Vol* v = r->getVol();
    cout << "\nReservation : " << r->getId() << "  (creee le " << r->getDateCreation() << ")\n"
         << "Statut      : " << versTexte(r->getStatut()) << "\n"
         << "Remboursement : " << versTexte(r->getRemboursement()) << "\n\n";
    r->getPassager()->afficher();
    cout << "\n  Vol       : " << v->getNumero() << " - " << v->getOrigine() << " -> "
         << v->getDestination() << "\n"
         << "  Depart    : " << v->getDateDepart() << "\n"
         << "  Prix      : " << utils::formaterMontant(v->getPrix()) << "\n";
    if (const Paiement* p = r->getPaiement())
        cout << "\n  Paiement  : " << p->getId() << " - " << utils::formaterMontant(p->getMontant())
             << " (" << versTexte(p->getMode()) << ") le " << p->getDate() << "\n";
    else
        cout << "\n  Paiement  : aucun\n";
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    Agence agence("Agence de voyage de Cotonou");
    bool quitter = false;
    try {
        while (!quitter) {
            afficherMenu();
            int choix = utils::lireEntier("Votre choix (1-10) : ", 1, 10);
            try {
                switch (choix) {
                    case 1:  actionEnregistrerPassager(agence); break;
                    case 2:  actionAjouterVol(agence); break;
                    case 3:  actionAfficherVolsDisponibles(agence); break;
                    case 4:  actionRechercherVol(agence); break;
                    case 5:  actionReserver(agence); break;
                    case 6:  actionPayer(agence); break;
                    case 7:  actionAnnuler(agence); break;
                    case 8:  actionConsulterReservations(agence); break;
                    case 9:  actionInfosReservation(agence); break;
                    case 10: quitter = true; break;
                }
            } catch (const utils::FinDeSaisie&) {
                throw;
            } catch (const exception& e) {
                cout << "\n[!] Operation refusee : " << e.what() << "\n";
            }
        }
    } catch (const utils::FinDeSaisie&) {
        cout << "\n(Fin de la saisie)\n";
    }
    cout << "\nMerci d'avoir utilise l'application. Au revoir !\n";
    return 0;
}
