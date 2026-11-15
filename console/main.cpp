// Version console : demande les fichiers et la clé, puis encode ou décode
// avec le cœur partagé (src/chiffrement.h).

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <system_error>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#include "chiffrement.h"

namespace fs = std::filesystem;

namespace
{
    enum class Mode
    {
        Encoder,
        Decoder,
    };

    // Levée quand l'entrée standard se ferme (Ctrl+Z / Ctrl+D, fin d'un fichier redirigé).
    struct EntreeFermee
    {
    };

    // Les textes du programme sont en UTF-8 : la console Windows doit l'être aussi.
    void configurerConsole()
    {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
#endif
    }

    std::string demanderLigne(const std::string& invite)
    {
        std::cout << invite;
        std::string ligne;
        if (!std::getline(std::cin, ligne))
        {
            throw EntreeFermee{};
        }
        if (!ligne.empty() && ligne.back() == '\r')
        {
            ligne.pop_back();
        }
        return ligne;
    }

    Mode demanderMode()
    {
        while (true)
        {
            const std::string choix = demanderLigne(
                "---------- M_E_N_U ----------\n"
                "Que voulez-vous faire ?\n"
                "1) Encoder un fichier.\n"
                "2) Décoder un fichier.\n"
                "Votre choix : ");
            if (choix == "1")
            {
                return Mode::Encoder;
            }
            if (choix == "2")
            {
                return Mode::Decoder;
            }
            std::cout << "Choix invalide : tapez 1 ou 2.\n\n";
        }
    }

    std::string demanderNomFichier(const std::string& invite)
    {
        while (true)
        {
            const std::string nom = demanderLigne(invite);
            if (!nom.empty())
            {
                return nom;
            }
            std::cout << "Le nom du fichier ne peut pas être vide.\n";
        }
    }

    // Les noms de fichiers saisis sont en UTF-8 (voir configurerConsole).
    fs::path cheminDepuis(const std::string& nom)
    {
        return fs::u8path(nom);
    }

    bool sontLeMemeFichier(const std::string& nomA, const std::string& nomB)
    {
        std::error_code erreur;
        // Renvoie false (sans exception) si l'un des deux fichiers n'existe pas encore.
        return fs::equivalent(cheminDepuis(nomA), cheminDepuis(nomB), erreur);
    }

    std::string demanderFichierEntree()
    {
        while (true)
        {
            const std::string nom = demanderNomFichier("Entrer le nom du fichier à traiter : ");
            std::error_code erreur;
            if (fs::is_regular_file(cheminDepuis(nom), erreur))
            {
                return nom;
            }
            std::cout << "Le fichier « " << nom << " » est introuvable.\n";
        }
    }

    std::string demanderFichierSortie(const std::string& fichierEntree)
    {
        while (true)
        {
            const std::string nom = demanderNomFichier("Entrer le nom du fichier à produire : ");
            if (!sontLeMemeFichier(fichierEntree, nom))
            {
                return nom;
            }
            std::cout << "Le fichier à produire doit être différent du fichier à traiter "
                         "(sinon son contenu serait effacé).\n";
        }
    }

    std::string demanderCle()
    {
        while (true)
        {
            const std::string cle = demanderLigne("Entrer la clé de traitement : ");
            const chiffrement::Erreur erreur = chiffrement::validerCle(cle);
            if (erreur == chiffrement::Erreur::Aucune)
            {
                return cle;
            }
            std::cout << "Erreur : " << chiffrement::messageErreur(erreur) << '\n';
        }
    }

    // Mode binaire : les octets sont lus et écrits tels quels, sans conversion des fins de ligne.
    bool lireFichier(const std::string& nom, std::string& contenu)
    {
        std::ifstream fichier(cheminDepuis(nom), std::ios::binary);
        if (!fichier)
        {
            return false;
        }
        std::ostringstream tampon;
        tampon << fichier.rdbuf();
        contenu = tampon.str();
        return !fichier.bad();
    }

    bool ecrireFichier(const std::string& nom, const std::string& contenu)
    {
        std::ofstream fichier(cheminDepuis(nom), std::ios::binary);
        fichier.write(contenu.data(), static_cast<std::streamsize>(contenu.size()));
        return static_cast<bool>(fichier);
    }

    int executer()
    {
        const Mode mode = demanderMode();
        const std::string fichierEntree = demanderFichierEntree();
        const std::string fichierSortie = demanderFichierSortie(fichierEntree);
        const std::string cle = demanderCle();

        std::string contenu;
        if (!lireFichier(fichierEntree, contenu))
        {
            std::cerr << "Erreur : impossible de lire le fichier « " << fichierEntree << " ».\n";
            return EXIT_FAILURE;
        }

        const chiffrement::Resultat resultat = (mode == Mode::Encoder)
            ? chiffrement::encoder(contenu, cle)
            : chiffrement::decoder(contenu, cle);
        if (!resultat.ok())
        {
            // Rien n'est écrit : on ne laisse jamais de fichier de sortie à moitié produit.
            std::cerr << "Erreur : " << chiffrement::messageErreur(resultat.erreur) << '\n';
            return EXIT_FAILURE;
        }

        if (!ecrireFichier(fichierSortie, resultat.texte))
        {
            std::cerr << "Erreur : impossible d'écrire le fichier « " << fichierSortie << " ».\n";
            return EXIT_FAILURE;
        }

        std::cout << (mode == Mode::Encoder ? "Encodage terminé." : "Décodage terminé.") << '\n';
        return EXIT_SUCCESS;
    }
}

int main()
{
    configurerConsole();
    try
    {
        return executer();
    }
    catch (const EntreeFermee&)
    {
        std::cerr << "\nSaisie interrompue.\n";
        return EXIT_FAILURE;
    }
}
