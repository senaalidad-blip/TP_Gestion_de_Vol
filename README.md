# Gestion des vols et réservations — Agence de voyage de Cotonou

Pourquoi perdre une journée entière en files d’attente pour une simple formalité de vol ? Aujourd’hui, réserver un billet ou finaliser un embarquement dans nos agences relève souvent du parcours du combattant : entre les processus manuels, l’engorgement des guichets et les lenteurs administratives, un simple déplacement peut immobiliser un passager pendant une demi-journée, voire plus. Face à cette perte de temps critique, j’ai conçu une solution logicielle intégrée dédiée à la digitalisation complète d’une agence de voyage à Cotonou. Cette application centralise et automatise en temps réel l’ensemble de la chaîne opérationnelle : de l’enregistrement des passagers et la planification des vols jusqu’à la gestion fluide des réservations, des paiements et des annulations.

## Fonctionnalités (menu principal)

1 Enregistrer un passager:  Nom, prénom, téléphone, n° de passeport (unique) 
2 Ajouter un vol: Code unique, départ, destination, date, heure, capacité, prix 
3 Afficher les vols disponibles: Vols ayant encore des places libres 
4 Rechercher un vol: Par destination, départ, date ou code 
5 Effectuer une réservation: Passager enregistré + vol disponible 
6 Enregistrer un paiement: Paiement intégral → réservation validée 
7 Annuler une réservation: Libère la place, remboursement si déjà payée 
8 Consulter les réservations: Toutes, ou celles d'un passager
9 Afficher les informations d'une réservation: Fiche détaillée 
10 Quitter 

## Règles de gestion implémentées

1. Un passager doit être **enregistré** avant de réserver.
2. Un vol **ne dépasse jamais sa capacité** (places provisoires + places confirmées).
3. Un passager **ne peut pas réserver deux fois le même vol** (tant que sa réservation n'est pas annulée ; après annulation, il peut réserver à nouveau).
4. Une réservation est liée à **un seul passager et un seul vol**.
5. Une réservation **annulée ne peut plus être payée**.
6. Une place n'est **définitive qu'après validation** : réservation `EN ATTENTE` = place provisoire ; après paiement → `VALIDEE` = place définitive.
7. **Remboursement** : annulation d'une réservation payée → statut `REMBOURSE`, 90 % du prix remboursé (10 % de frais, constante `Reservation::TAUX_REMBOURSEMENT`) ; annulation d'une réservation non payée → `NON APPLICABLE`.

## Architecture du Système et Modèle Orienté Objet

L'application repose sur une architecture orientée objet structurée autour de cinq classes principales permettant de modéliser et d'automatiser l'ensemble des opérations de l'agence de voyage :

### 1. Les Classes, Attributs et Méthodes

* **Passager**
  * **Attributs :** `id`, `nom`, `prenom`, `telephone`, `email`
  * **Méthodes :** Constructeur, getters/setters, `afficher()`

* **Vol**
  * **Attributs :** `numero`, `depart`, `destination`, `date`, `capacite`, `prix`, `nbReservations`
  * **Méthodes :** `estComplet()`, `placesDisponibles()`, `afficher()`

* **Reservation**
  * **Attributs :** `id`, `passager`, `vol`, `statut` (`EN_ATTENTE`, `VALIDEE`, `ANNULEE`), `date`
  * **Méthodes :** `valider()`, `annuler()`, `afficher()`

* **Paiement**
  * **Attributs :** `id`, `reservation`, `montant`, `date`, `statutRemboursement` (`AUCUN`, `A_REMBOURSER`, `REMBOURSE`)
  * **Méthodes :** `effectuer()`, `rembourser()`

* **Agence (Classe de gestion)**
  * **Attributs :** Conteneurs (`vector`) regroupant les listes de passagers, de vols, de réservations et de paiements.
  * **Méthodes :** `enregistrerPassager()`, `ajouterVol()`, `reserver()`, `payer()`, `annulerReservation()`, `rechercherVol()`

### 2. Associations et Multiplicités

* **Passager `1 ── 0..*` Réservation :** Un passager peut effectuer zéro ou plusieurs réservations.
* **Vol `1 ── 0..*` Réservation :** Un vol peut être associé à zéro ou plusieurs réservations.
* **Réservation `1 ── 0..1` Paiement :** Une réservation est associée à au plus un paiement (en attente de règlement ou déjà payée).
* **Agence `1 ◆── 0..*` Entités :** La classe `Agence` entretient une relation de composition avec les autres classes ; elle centralise et gère l'ensemble des instances de passagers, vols, réservations et paiements.

## Architecture

| Fichier | Rôle |
|---------|------|
| `Passager.h` | Classe `Passager` |
| `Vol.h` | Classe `Vol` (gestion des places) |
| `Paiement.h` | Classe `Paiement` + enum `ModePaiement` |
| `Reservation.h` | Classe `Reservation` + enums de statut ; porte les règles de validation/annulation |
| `Agence.h / Agence.cpp` | Classe `Agence` : collections `std::vector<std::shared_ptr<...>>` et opérations métier |
| `Utils.h` | Saisies sécurisées (entiers, dates, téléphone…) et formatage des montants |
| `main.cpp` | Menu et interface console |
| `diagramme_classes.puml` | Diagramme de classes UML (PlantUML) |

Principes POO : **encapsulation** (attributs privés), **constructeurs** avec validation, **accesseurs**/mutateurs,
**associations** par `shared_ptr` (Reservation → Passager, Vol, Paiement), **collections** `std::vector`,
**exceptions** (`std::runtime_error`, `std::invalid_argument`) pour les opérations interdites, saisies
contrôlées en boucle jusqu'à validité.

## Compilation et exécution

Prérequis : un compilateur C++17 (g++ ≥ 7, clang, MinGW).

```bash
# Avec make
make
./agence            # Windows : agence.exe

# Ou directement
g++ -std=c++17 -Wall -Wextra -o agence main.cpp Agence.cpp
```

Charger des données de démonstration (3 passagers, 4 vols) :

```bash
./agence --demo
```

## Conception : Diagramme de classes UML

Le diagramme de classes (`UML_AgenceVoyage.pdf`) modélise le système de gestion des vols et des réservations de l'agence de voyage de Cotonou. Il est organisé autour d'une classe centrale, `AgenceVoyage`, qui coordonne quatre classes métier (`Passager`, `Vol`, `Reservation`, `Paiement`) et trois énumérations de statuts.

### Classes principales

| Classe | Rôle | Attributs clés | Méthodes principales |
|---|---|---|---|
| `AgenceVoyage` | Classe centrale qui regroupe les données et pilote les opérations | `passagers`, `vols`, `reservations` (`std::vector`) | `enregistrerPassager()`, `ajouterVol()`, `rechercherVol()`, `effectuerReservation()`, `enregistrerPaiement()`, `annulerReservation()` |
| `Passager` | Représente un client de l'agence | `id`, `nom`, `prenom`, `telephone`, `email` | `getId()`, `afficher()`, `modifierCoordonnees()` |
| `Vol` | Représente un vol programmé | `numero`, `origine`, `destination`, `dateDepart`, `capacite`, `placesReservees` | `estDisponible()`, `ajouterReservation()`, `libererPlace()`, `afficher()` |
| `Reservation` | Lie un passager à un vol | `numero`, `dateReservation`, `statut`, `passager`, `vol` | `valider()`, `annuler()`, `estPayable()`, `afficherDetails()` |
| `Paiement` | Représente le règlement d'une réservation | `reference`, `montant`, `datePaiement`, `statut`, `remboursement` | `effectuer()`, `rembourser()`, `afficher()` |

### Énumérations

- **`StatutReservation`** : `EN_ATTENTE`, `VALIDEE`, `PAYEE`, `ANNULEE`
- **`StatutPaiement`** : `EN_ATTENTE`, `VALIDE`, `REFUSE`, `REMBOURSE`
- **`StatutRemboursement`** : `AUCUN`, `A_REMBOURSER`, `REMBOURSE`

### Relations entre les classes

| Relation | Cardinalités | Signification |
|---|---|---|
| `AgenceVoyage` « gère » `Passager` | 1 → 0..* | Une agence gère plusieurs passagers. |
| `AgenceVoyage` « programme » `Vol` | 1 → 0..* | Une agence programme plusieurs vols. |
| `AgenceVoyage` « enregistre » `Reservation` | 1 → 0..* | Une agence enregistre plusieurs réservations. |
| `Reservation` « concerne » `Passager` | 0..* → 1 | Une réservation concerne un seul passager, qui peut en avoir plusieurs. |
| `Reservation` « porte sur » `Vol` | 0..* → 1 | Une réservation porte sur un seul vol, qui peut en recevoir plusieurs. |
| `Reservation` « donne lieu à » `Paiement` | 1 → 0..1 | Une réservation donne lieu à au plus un paiement. |

Les attributs `statut` et `remboursement` de `Reservation` et `Paiement` dépendent des énumérations correspondantes.

### Choix de conception

- **Encapsulation** : tous les attributs sont privés (`-`) et les méthodes publiques (`+`).
- **Centralisation** : `AgenceVoyage` est le point d'entrée des fonctionnalités du menu (enregistrement, recherche, réservation, paiement, annulation).
- **Gestion de la capacité** : `Vol` suit ses `placesReservees` par rapport à sa `capacite`. `estDisponible()` et `libererPlace()` gèrent respectivement la disponibilité et la libération d'une place après annulation.
- **Cycle de vie d'une réservation** : elle passe de `EN_ATTENTE` à `VALIDEE`, puis à `PAYEE` une fois le paiement effectué, ou à `ANNULEE`.
- **Suivi des paiements** : `Paiement` gère son propre statut et l'état d'un éventuel remboursement, ce qui permet de traiter une annulation après paiement.
- **Associations par pointeurs** : `Reservation` référence `Passager` et `Vol` par des pointeurs, ce qui évite de dupliquer les objets.
