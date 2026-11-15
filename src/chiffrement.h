// Cœur de l'encodeur/décodeur : l'algorithme seul, sans fichiers ni console.
// Il est partagé par la version console (console/main.cpp) et la version web
// (web/bindings.cpp), et testé par tests/tests_chiffrement.cpp.
#pragma once

#include <cstddef>
#include <string>

namespace chiffrement
{
    // Caractères pris en charge : les caractères ASCII imprimables, de l'espace (32) au tilde (126).
    constexpr int PREMIER_IMPRIMABLE = 32;
    constexpr int DERNIER_IMPRIMABLE = 126;
    constexpr int NOMBRE_IMPRIMABLES = DERNIER_IMPRIMABLE - PREMIER_IMPRIMABLE + 1;  // 95

    // Règles de la clé : de 5 à 20 caractères imprimables, sans espace.
    constexpr std::size_t TAILLE_CLE_MIN = 5;
    constexpr std::size_t TAILLE_CLE_MAX = 20;

    // Le décalage appliqué par chaque caractère de la clé se mesure à partir de cette lettre.
    constexpr char REFERENCE_DECALAGE = 'a';

    enum class Erreur
    {
        Aucune,
        CleTailleInvalide,
        CleCaractereInvalide,
        TexteCaractereNonSupporte,
        TexteNonEncode,
    };

    struct Resultat
    {
        Erreur erreur = Erreur::Aucune;
        std::string texte;  // texte encodé ou décodé, vide en cas d'erreur

        bool ok() const { return erreur == Erreur::Aucune; }
    };

    // Vérifie la clé ; renvoie Erreur::Aucune si elle est valide.
    Erreur validerCle(const std::string& cle);

    // Chaque caractère imprimable est décalé selon le caractère courant de la clé
    // (réutilisée en boucle) ; les retours à la ligne (\n et \r) sont conservés tels quels.
    Resultat encoder(const std::string& texte, const std::string& cle);
    Resultat decoder(const std::string& texte, const std::string& cle);

    // Message en français à afficher pour une erreur.
    std::string messageErreur(Erreur erreur);
}
