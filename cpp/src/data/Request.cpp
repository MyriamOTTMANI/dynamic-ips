// Adapté pour le projet Bourse de Fret — Stage Polytechnique Montréal

#include "Request2.h"
#include "Vehicle.h"

unsigned int Request::requestCount_ = 0;

// ---------------------------------------------------------------------------
// Constructeur
// ---------------------------------------------------------------------------
Request::Request(int pickUpID, int dropOffID,
                 float requestTime, float latestPickup,
                 float earlyDrop, float latestDrop,
                 int tonnage, int corridorID,
                 float prixPropose,
                 bool requiresADR, bool requiresCabotage,
                 Preference_transport preferenceTransport,
                 Type_paiement typePaiement) :
        requestID_(requestCount_++),
        PickUpID_(pickUpID), DropOffID_(dropOffID),
        corridorID_(corridorID),
        requestTime_(requestTime), latestPickup_(latestPickup),
        earlyDrop_(earlyDrop), latestDrop_(latestDrop),
        Tonnage_marchandise_(tonnage),
        Prix_propose_(prixPropose),
        requiresADR_(requiresADR), requiresCabotage_(requiresCabotage),
        preference_transport_(preferenceTransport),
        typePaiement_(typePaiement) {

    name_               = std::to_string(requestID_);
    prixMoyen_corridor_ = 0.0f;

    // Rolling horizon
    batchID_            = -1;
    penaltyBatchCount_  = 0;
    penalty_            = 0.0f;
    isLocked_           = false;
    requestStatus_      = NO_ACTION;

    // Column generation
    dual_               = 0.0f;
    lastDual_           = 0.0f;
    taskIndex_          = -1;
    taskIndexLabel_     = -1;

    // Tracking simulation (LARGE_CONSTANT = non encore défini)
    assignTime_         = constants::LARGE_CONSTANT;
    commitTime_         = constants::LARGE_CONSTANT;
    committedPickTime_  = constants::LARGE_CONSTANT;
    pickTime_           = constants::LARGE_CONSTANT;
    dropTime_           = constants::LARGE_CONSTANT;
    allocVehicleID_     = constants::LARGE_CONSTANT;
    solVehicleID_       = constants::LARGE_CONSTANT;

    minTravelTime_      = 0.0f;
}

// ---------------------------------------------------------------------------
// Destructeur
// ---------------------------------------------------------------------------
Request::~Request() {
}

// ---------------------------------------------------------------------------
// Affichage
// ---------------------------------------------------------------------------
std::string Request::toString() const {
    std::stringstream repStr;
    repStr << std::left;
    repStr << "# REQUEST ( " << requestID_ << " )" << std::endl;
    repStr << "#\t" << std::setw(24) << "- PICKUP_ID"   << " : " << PickUpID_             << std::endl;
    repStr << "#\t" << std::setw(24) << "- DROPOFF_ID"  << " : " << DropOffID_            << std::endl;
    repStr << "#\t" << std::setw(24) << "- TONNAGE"     << " : " << Tonnage_marchandise_  << std::endl;
    repStr << "#\t" << std::setw(24) << "- PRIX"        << " : " << Prix_propose_         << std::endl;
    repStr << "#\t" << std::setw(24) << "- STATUS"      << " : " << requestStatus_        << std::endl;
    repStr << "#" << std::endl;
    return repStr.str();
}

// ---------------------------------------------------------------------------
// Getters / Setters
// ---------------------------------------------------------------------------
unsigned int Request::getRequestId() const { return requestID_; }

void Request::setMinTravelTime(float minTravelTime) {
    minTravelTime_ = minTravelTime;
}

// ---------------------------------------------------------------------------
// Rolling horizon : mise à jour de la pénalité
// Appelé au début de chaque nouveau batch
// ---------------------------------------------------------------------------
void Request::updatePenalty(int currentBatchID, const PParameters& parameters) {
    if (batchID_ >= 0) {
        penaltyBatchCount_ = currentBatchID - batchID_;
    }
    penalty_ = parameters->deltaPram_ * static_cast<float>(penaltyBatchCount_);
}

// ---------------------------------------------------------------------------
// Column generation : contribution au coût réduit
// ---------------------------------------------------------------------------
float Request::reducedCostContribution() const {
    return Prix_propose_ - dual_;
}

// ---------------------------------------------------------------------------
// Compatibilité avec un camion (filtre graphe)
// ---------------------------------------------------------------------------
bool Request::isCompatibleWith(const Vehicle& truck) const {
    // À compléter selon les champs de Vehicle.h
    // Exemple de vérifications minimales :
    //   - capacité suffisante
    //   - type de camion compatible avec type de marchandise
    //   - certifications ADR si nécessaire
    return true;
}
