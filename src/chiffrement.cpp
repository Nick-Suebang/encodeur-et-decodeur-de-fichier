#include "chiffrement.h"

namespace chiffrement
{
    namespace
    {
        enum class Sens
        {
            Encoder = 1,
            Decoder = -1,
        };

        bool estImprimable(unsigned char c)
        {
            return c >= PREMIER_IMPRIMABLE && c <= DERNIER_IMPRIMABLE;
        }

        bool estRetourLigne(unsigned char c)
        {
            return c == '\n' || c == '\r';
        }

        // Décale un caractère imprimable en restant dans la plage 32-126 (arithmétique modulaire),
        // quel que soit le signe du décalage.
        char decaler(unsigned char c, int decalage)
        {
            const int position = (c - PREMIER_IMPRIMABLE + decalage) % NOMBRE_IMPRIMABLES;
            const int positionPositive = (position + NOMBRE_IMPRIMABLES) % NOMBRE_IMPRIMABLES;
            return static_cast<char>(PREMIER_IMPRIMABLE + positionPositive);
        }

        Resultat transformer(const std::string& texte, const std::string& cle, Sens sens)
        {
            const Erreur erreurCle = validerCle(cle);
            if (erreurCle != Erreur::Aucune)
            {
                return { erreurCle, "" };
            }

            std::string sortie;
            sortie.reserve(texte.size());
            std::size_t positionCle = 0;

            for (const unsigned char c : texte)
            {
                if (estRetourLigne(c))
                {
                    sortie += static_cast<char>(c);
                }
                else if (estImprimable(c))
                {
                    const int decalage = cle[positionCle] - REFERENCE_DECALAGE;
                    sortie += decaler(c, decalage * static_cast<int>(sens));
                    positionCle = (positionCle + 1) % cle.size();
                }
                else
                {
                    return { sens == Sens::Encoder ? Erreur::TexteCaractereNonSupporte : Erreur::TexteNonEncode, "" };
                }
            }
            return { Erreur::Aucune, sortie };
        }
    }

    Erreur validerCle(const std::string& cle)
    {
        // Les caractères sont vérifiés en premier : un caractère accentué occupe plusieurs octets
        // et fausserait le message sur la longueur.
        for (const unsigned char c : cle)
        {
            if (!estImprimable(c) || c == ' ')
            {
                return Erreur::CleCaractereInvalide;
            }
        }
        if (cle.size() < TAILLE_CLE_MIN || cle.size() > TAILLE_CLE_MAX)
        {
            return Erreur::CleTailleInvalide;
        }
        return Erreur::Aucune;
    }

    Resultat encoder(const std::string& texte, const std::string& cle)
    {
        return transformer(texte, cle, Sens::Encoder);
    }

    Resultat decoder(const std::string& texte, const std::string& cle)
    {
        return transformer(texte, cle, Sens::Decoder);
    }

    std::string messageErreur(Erreur erreur)
    {
        switch (erreur)
        {
        case Erreur::Aucune:
            return "";
        case Erreur::CleTailleInvalide:
            return "La clé doit contenir entre " + std::to_string(TAILLE_CLE_MIN) + " et "
                   + std::to_string(TAILLE_CLE_MAX) + " caractères.";
        case Erreur::CleCaractereInvalide:
            return "La clé ne doit contenir que des caractères anglais imprimables, sans espace ni accent.";
        case Erreur::TexteCaractereNonSupporte:
            return "Le texte contient des caractères non supportés (accents, émojis, tabulations…) : "
                   "seuls les caractères anglais imprimables et les retours à la ligne sont acceptés.";
        case Erreur::TexteNonEncode:
            return "Ce texte ne peut pas provenir de l'encodeur : il contient des caractères "
                   "qu'un texte encodé ne contient jamais.";
        }
        return "Erreur inconnue.";
    }
}
