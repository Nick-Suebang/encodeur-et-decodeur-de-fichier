// Version web : expose le cœur partagé (src/chiffrement.h) à JavaScript.
// Compilé en WebAssembly avec Emscripten (voir web/build.sh).

#include <string>

#include <emscripten/bind.h>

#include "chiffrement.h"

namespace
{
    // Forme renvoyée à JavaScript : { ok, texte, erreur }.
    struct ResultatJs
    {
        bool ok;
        std::string texte;
        std::string erreur;
    };

    ResultatJs versJs(const chiffrement::Resultat& resultat)
    {
        return { resultat.ok(), resultat.texte, chiffrement::messageErreur(resultat.erreur) };
    }

    ResultatJs encoder(const std::string& texte, const std::string& cle)
    {
        return versJs(chiffrement::encoder(texte, cle));
    }

    ResultatJs decoder(const std::string& texte, const std::string& cle)
    {
        return versJs(chiffrement::decoder(texte, cle));
    }

    // Message d'erreur sur la clé, ou chaîne vide si elle est valide.
    std::string verifierCle(const std::string& cle)
    {
        return chiffrement::messageErreur(chiffrement::validerCle(cle));
    }
}

EMSCRIPTEN_BINDINGS(encodeur)
{
    emscripten::value_object<ResultatJs>("Resultat")
        .field("ok", &ResultatJs::ok)
        .field("texte", &ResultatJs::texte)
        .field("erreur", &ResultatJs::erreur);

    emscripten::function("encoder", &encoder);
    emscripten::function("decoder", &decoder);
    emscripten::function("verifierCle", &verifierCle);
}
