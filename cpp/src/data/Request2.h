// Adapté pour le projet Bourse de Fret — Stage Polytechnique Montréal
// Système de recommandation FTL (Full Truck Load) — CloudFret
// Basé sur le repo dynamic-ips (Elahe_Amiri), modifié pour PPDSP + rolling horizon

#ifndef REQUEST2_H
#define REQUEST2_H

#include "utilities/MyTools.h"
#include "Parameters.h"

class Vehicle;   
// ---------------------------------------------------------------------------
// Request class
// Représente une marchandise disponible sur la bourse de fret.
// En FTL : un camion transporte une seule marchandise à la fois par segment.
// La capacité est un FILTRE GRAPHE (≥ 80%), pas une ressource dans l'étiquette.
// ---------------------------------------------------------------------------

class Request {
private:
    const unsigned int requestID_;          // ID unique de la marchandise

public:
    static unsigned int requestCount_;      // Compteur global de marchandises
    std::string name_;                      // Nom/référence de la marchandise

    // -----------------------------------------------------------------------
    // LOCALISATION nœuds du graphe ESPPRC
    // Chaque Request = un nœud (tâche) dans le graphe abstrait du pricing.
    // Un arc (i→j) = trajet entre livraison de i et pickup de j.
    // -----------------------------------------------------------------------
    int PickUpID_;                          // ID du lieu de collecte
    int DropOffID_;                         // ID du lieu de livraison
    int corridorID_;                        // Corridor : Casa↔Tanger, Casa↔Agadir, etc.

    // -----------------------------------------------------------------------
    // TEMPOREL  time windows et rolling horizon
    // -----------------------------------------------------------------------
    float requestTime_;                     // Heure à laquelle la marchandise est disponible (release time)
    float earlyDrop_;                       // TW livraison  début
    float latestDrop_;                      // TW livraison  fin
    float minTravelTime_;                   // Durée minimale du trajet direct pickup→dropoff
    float latestPickup_;   // deadline de collecte — au-delà la marchandise est expirée
    // -----------------------------------------------------------------------
    // FINANCIER objectif du master problem
    // Note : prix non fixe sur CloudFret (enchères).
    // Prix_proposé = valeur de l'enchère en cours ou prix moyen historique.
    // -----------------------------------------------------------------------
    float Prix_propose_;                    // Prix proposé pour cette marchandise
    float prixMoyen_corridor_;              // Prix moyen historique sur ce corridor (estimateur enchère)
    Type_paiement typePaiement_;            // Paiement : Avance, à la livraison, crédit, etc.

    // -----------------------------------------------------------------------
    // PHYSIQUE  filtres graphe 
    // -----------------------------------------------------------------------
    int    Tonnage_marchandise_;            // Poids en kg (filtre : poids/capacité_camion ≥ 80%)
    bool   requiresADR_;                   // Matières dangereuses (certification + incompatibilités)
    bool   requiresCabotage_;             // Transport transfrontalier (visa chauffeur requis)
    Preference_transport preference_transport_; 

    // -----------------------------------------------------------------------
    // ROLLING HORIZON  gestion des batches
    // -----------------------------------------------------------------------
    int    batchID_;                        // ID du batch dans lequel la marchandise a été intégrée
    int    penaltyBatchCount_;             // Nombre de batches écoulés sans être servie
                                           // → utilisé par updatePenalty() pour calculer penalty_
    float  penalty_;                       // Pénalité effective (terme -pen_i * z_i dans le master)
                                           // = f(penaltyBatchCount_, parameters), mise à jour à chaque batch
    bool   isLocked_;                      // true = réservée par un camion en cours de négociation
    RequestStatus requestStatus_;          // Statut : NO_ACTION, ON_BOARD, COMPLETED, REJECTED

    // -----------------------------------------------------------------------
    // COLUMN GENERATION  lien avec le master problem (Set Packing)
    // Ces champs sont mis à jour à chaque itération de la col gen.
    // -----------------------------------------------------------------------
    float InitialDual_;                         // when in parameters we use penalties as duals, we save previous duals in it
    float  dual_;                           // μᵢ : variable duale courante du master problem
    float  lastDual_;                       // μᵢ du batch précédent (warm start)
    int    taskIndex_;                      // Indice de la ligne de cette marchandise dans la matrice RMP
    int    taskIndexLabel_;                // Indice dans le graphe du sous-problème (pricing)
    float minDual_;                             // minimum dual value among all epochs
    float maxDual_;                             // maximum dual value among all epochs
    float marginalCost_;                        // marginal cost of inserting this request in a vehicle route in the greedy method
    
    // -----------------------------------------------------------------------
    // SUIVI  tracking de l'assignation et de l'engagement
    // -----------------------------------------------------------------------
    float  assignTime_;                     // Heure de première assignation à un plan
    float  commitTime_;                     // Heure d'engagement ferme (annonce au client)
    float  committedPickTime_;             // Heure de pickup annoncée au client
    float  pickTime_;                      // Heure réelle de pickup (simulation)
    float  dropTime_;                      // Heure réelle de livraison (simulation)
    int    allocVehicleID_;               // ID du camion qui sert cette marchandise
    int    solVehicleID_;                 // ID du camion dans la solution courante


    // -----------------------------------------------------------------------
    // Constructeur / Destructeur
    // -----------------------------------------------------------------------

    Request(int pickUpID, int dropOffID,
            float requestTime, float latestPickup,
            float earlyDrop, float latestDrop,
            int tonnage, int corridorID,
            float prixPropose,
            bool requiresADR, bool requiresCabotage,
            Preference_transport preferenceTransport,
            Type_paiement typePaiement);

    virtual ~Request();

    // -----------------------------------------------------------------------
    // Méthodes utilitaires
    // -----------------------------------------------------------------------
    std::string toString() const;

    unsigned int getRequestId() const;

    void setMinTravelTime(float minTravelTime);

    // Met à jour la pénalité en fonction du nombre de batches sans assignation
    // Appelé au début de chaque nouveau batch (rolling horizon)
    void updatePenalty(int currentBatchID, const PParameters& parameters);

    // Vérifie la compatibilité avec un camion donné (filtre graphe)
    // Retourne false si incompatible (ne doit pas apparaître dans le graphe de ce camion)
    bool isCompatibleWith(const Vehicle& truck) const;

    // Contribution de cette marchandise au coût réduit d'une route
    // contribution = Prix_propose_ - dual_
    float reducedCostContribution() const;
};

// Comparateur sur requestID (utilisé dans les conteneurs)
inline bool operator==(const PRequest& lhs, const PRequest& rhs) {
    return (lhs->getRequestId() == rhs->getRequestId());
}

#endif // REQUEST2_H