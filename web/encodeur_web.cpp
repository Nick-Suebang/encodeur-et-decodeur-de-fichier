// Version web de l'encodeur/décodeur (compilée en WebAssembly avec Emscripten).
//
// La logique est la même que dans main.cpp ; seules les entrées/sorties changent :
// au lieu de lire et d'écrire des fichiers sur le disque avec cin/ifstream/ofstream,
// les fonctions reçoivent le texte et la clé depuis la page web et renvoient le résultat.

#include <string>
#include <emscripten/bind.h>

using namespace std;

struct Resultat
{
    bool ok;
    string texte;   // texte encodé ou décodé (si ok)
    string erreur;  // message d'erreur (si !ok)
};

static int tailleCle(const string& cle)
{
    return static_cast<int>(cle.size());
}

// Même règles que verificationCle() dans main.cpp, mais renvoie un message
// au lieu de redemander la clé dans la console. Chaîne vide = clé valide.
string verifierCle(const string& cle)
{
    if (tailleCle(cle) < 5 || tailleCle(cle) > 20)
    {
        return "Votre clé doit contenir entre 5 et 20 caractères.";
    }
    for (unsigned char c : cle)
    {
        if (c < 33 || c > 126)
        {
            return "Votre clé ne doit contenir que des caractères anglais (sans espace ni accent).";
        }
    }
    return "";
}

Resultat encoder(const string& texte, const string& cle)
{
    string erreurCle = verifierCle(cle);
    if (!erreurCle.empty())
    {
        return { false, "", erreurCle };
    }

    string sorti;
    sorti.reserve(texte.size());
    int positionLettreCle{ 0 };

    for (unsigned char lettre : texte)
    {
        // caractère imprimable entre 32 et 126
        if (lettre >= 32 && lettre <= 126)
        {
            int decalage{ cle[positionLettreCle] - 'a' };
            int lettreEncodee{ lettre + decalage };
            while (lettreEncodee > 126)
            {
                lettreEncodee -= 95;
            }
            // Ajout par rapport à main.cpp : une clé avec des majuscules, chiffres ou
            // symboles donne un décalage négatif ; on reste dans la plage 32-126.
            while (lettreEncodee < 32)
            {
                lettreEncodee += 95;
            }
            sorti += char(lettreEncodee);
            positionLettreCle = (positionLettreCle + 1) % tailleCle(cle);
        }
        // retour à la ligne : conservé tel quel
        else if (lettre == 10 || lettre == 13)
        {
            sorti += char(lettre);
        }
        else
        {
            return { false, "", "Code erreur -1 : certains caractères du texte ne sont pas supportés "
                                "(accents, émojis, tabulations…). Seuls les caractères anglais sont acceptés." };
        }
    }
    return { true, sorti, "" };
}

Resultat decoder(const string& texte, const string& cle)
{
    string erreurCle = verifierCle(cle);
    if (!erreurCle.empty())
    {
        return { false, "", erreurCle };
    }

    string sorti;
    sorti.reserve(texte.size());
    int positionLettreCle{ 0 };

    for (unsigned char lettre : texte)
    {
        // retour à la ligne : conservé tel quel
        if (lettre == 10 || lettre == 13)
        {
            sorti += char(lettre);
        }
        else if (lettre >= 32 && lettre <= 126)
        {
            int decalage{ cle[positionLettreCle] - 'a' };
            int lettreDecodee{ lettre - decalage };
            while (lettreDecodee < 32)
            {
                lettreDecodee += 95;
            }
            // symétrique de l'ajout fait à l'encodage
            while (lettreDecodee > 126)
            {
                lettreDecodee -= 95;
            }
            sorti += char(lettreDecodee);
            positionLettreCle = (positionLettreCle + 1) % tailleCle(cle);
        }
        else
        {
            return { false, "", "Ce texte ne provient pas de l'encodeur : il contient des caractères "
                                "qu'un texte encodé ne peut pas contenir." };
        }
    }
    return { true, sorti, "" };
}

// Rend les fonctions C++ appelables depuis JavaScript.
EMSCRIPTEN_BINDINGS(encodeur)
{
    emscripten::value_object<Resultat>("Resultat")
        .field("ok", &Resultat::ok)
        .field("texte", &Resultat::texte)
        .field("erreur", &Resultat::erreur);

    emscripten::function("encoder", &encoder);
    emscripten::function("decoder", &decoder);
    emscripten::function("verifierCle", &verifierCle);
}
