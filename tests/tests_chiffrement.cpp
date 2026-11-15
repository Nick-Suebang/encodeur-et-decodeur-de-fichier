// Tests du cœur (src/chiffrement.h), sans bibliothèque externe.
// Lancement : sh tests/run.sh

#include <cstdlib>
#include <iostream>
#include <random>
#include <string>

#include "chiffrement.h"

using chiffrement::decoder;
using chiffrement::encoder;
using chiffrement::Erreur;
using chiffrement::validerCle;

namespace
{
    int echecs = 0;
    int verifications = 0;

    void verifier(bool condition, const std::string& description)
    {
        ++verifications;
        if (!condition)
        {
            ++echecs;
            std::cerr << "ÉCHEC : " << description << '\n';
        }
    }

    // Algorithme d'encodage de la version console d'origine (main.cpp avant la refonte),
    // valable pour les clés en minuscules : sert à vérifier que les résultats n'ont pas changé.
    std::string encoderCommeOrigine(const std::string& texte, const std::string& cle)
    {
        std::string sortie;
        std::size_t position = 0;
        for (const char c : texte)
        {
            if (c == '\n' || c == '\r')
            {
                sortie += c;
                continue;
            }
            int encode = c + (cle[position] - 'a');
            while (encode > 126)
            {
                encode -= 95;
            }
            sortie += static_cast<char>(encode);
            position = (position + 1) % cle.size();
        }
        return sortie;
    }

    std::string chaineAleatoire(std::mt19937& generateur, std::size_t taille, int min, int max)
    {
        std::uniform_int_distribution<int> distribution(min, max);
        std::string chaine;
        for (std::size_t i = 0; i < taille; ++i)
        {
            chaine += static_cast<char>(distribution(generateur));
        }
        return chaine;
    }

    bool estEncodageValide(const std::string& texte)
    {
        for (const unsigned char c : texte)
        {
            if (c != '\n' && c != '\r' && (c < 32 || c > 126))
            {
                return false;
            }
        }
        return true;
    }

    void testerValidationCle()
    {
        verifier(validerCle("abcde") == Erreur::Aucune, "clé de 5 caractères acceptée");
        verifier(validerCle("abcdefghijklmnopqrst") == Erreur::Aucune, "clé de 20 caractères acceptée");
        verifier(validerCle("Secret!42") == Erreur::Aucune, "clé avec majuscules, chiffres et symboles acceptée");
        verifier(validerCle("abcd") == Erreur::CleTailleInvalide, "clé de 4 caractères refusée");
        verifier(validerCle("abcdefghijklmnopqrstu") == Erreur::CleTailleInvalide, "clé de 21 caractères refusée");
        verifier(validerCle("") == Erreur::CleTailleInvalide, "clé vide refusée");
        verifier(validerCle("ab cde") == Erreur::CleCaractereInvalide, "clé avec espace refusée");
        verifier(validerCle("clé123") == Erreur::CleCaractereInvalide, "clé avec accent refusée");
        verifier(validerCle("abc\tdef") == Erreur::CleCaractereInvalide, "clé avec tabulation refusée");
        verifier(validerCle("éééééééééééééééééééé") == Erreur::CleCaractereInvalide,
                 "20 accents : erreur de caractère, pas de longueur");
    }

    void testerValeursConnues()
    {
        verifier(encoder("Bonjour", "ab").erreur == Erreur::CleTailleInvalide, "clé trop courte refusée à l'encodage");
        verifier(encoder("Bonjour", "abcde").texte == "Bppmsus", "valeur connue : Bonjour / abcde");
        verifier(encoder("Bonjour ! Ceci est un message secret.", "portfolio").texte
                     == "Q} }t$}(//Qvvn.p{#/$ 3rs~{ovs1'jq}m#=",
                 "valeur connue : exemple de la page web");
        verifier(encoder("", "abcde").ok() && encoder("", "abcde").texte.empty(), "texte vide accepté");
    }

    void testerRetoursLigne()
    {
        const std::string texte = "ligne 1\r\nligne 2\nligne 3\r\n";
        const auto encode = encoder(texte, "Secret!42");
        verifier(encode.ok(), "texte avec CRLF et LF encodé");
        verifier(decoder(encode.texte, "Secret!42").texte == texte, "retours à la ligne conservés");
    }

    void testerErreursTexte()
    {
        verifier(encoder("café", "abcde").erreur == Erreur::TexteCaractereNonSupporte, "accent refusé à l'encodage");
        verifier(encoder("a\tb", "abcde").erreur == Erreur::TexteCaractereNonSupporte, "tabulation refusée");
        verifier(encoder("a\tb", "abcde").texte.empty(), "aucun résultat partiel en cas d'erreur");
        verifier(decoder("a\tb", "abcde").erreur == Erreur::TexteNonEncode, "texte non encodé refusé au décodage");
        verifier(decoder("abc", "x").erreur == Erreur::CleTailleInvalide, "clé invalide refusée au décodage");
    }

    // Ancien bug : avec une clé majuscule, '*' devenait un retour à la ligne et le décodage échouait.
    void testerAncienBugCleMajuscule()
    {
        const auto encode = encoder("a*b", "AAAAA");
        verifier(encode.texte.find('\n') == std::string::npos, "'*' n'est plus encodé en retour à la ligne");
        verifier(decoder(encode.texte, "AAAAA").texte == "a*b", "aller-retour correct avec une clé majuscule");
    }

    void testerAleatoire()
    {
        std::mt19937 generateur(2333658);  // graine fixe : tests reproductibles
        std::uniform_int_distribution<int> tailleCle(5, 20);
        for (int i = 0; i < 2000; ++i)
        {
            const std::string texte = chaineAleatoire(generateur, 300, 32, 127);  // 127 remplacé par \n
            std::string texteAvecLignes = texte;
            for (char& c : texteAvecLignes)
            {
                if (c == 127)
                {
                    c = '\n';
                }
            }

            const std::string cleMinuscule = chaineAleatoire(generateur, tailleCle(generateur), 'a', 'z');
            const auto encodeMinuscule = encoder(texteAvecLignes, cleMinuscule);
            verifier(encodeMinuscule.texte == encoderCommeOrigine(texteAvecLignes, cleMinuscule),
                     "clé minuscule : même résultat que la version d'origine (" + cleMinuscule + ")");

            const std::string cleQuelconque = chaineAleatoire(generateur, tailleCle(generateur), 33, 126);
            const auto encode = encoder(texteAvecLignes, cleQuelconque);
            verifier(encode.ok() && estEncodageValide(encode.texte),
                     "clé quelconque : sortie imprimable (" + cleQuelconque + ")");
            verifier(decoder(encode.texte, cleQuelconque).texte == texteAvecLignes,
                     "clé quelconque : aller-retour exact (" + cleQuelconque + ")");
        }
    }
}

int main()
{
    testerValidationCle();
    testerValeursConnues();
    testerRetoursLigne();
    testerErreursTexte();
    testerAncienBugCleMajuscule();
    testerAleatoire();

    std::cout << (verifications - echecs) << " / " << verifications << " vérifications réussies.\n";
    return echecs == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
