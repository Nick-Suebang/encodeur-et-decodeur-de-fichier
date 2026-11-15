# Projet d'Encodage et Décodage de Fichiers

Ce projet est un programme en C++ qui permet d'encoder et de décoder des fichiers texte en utilisant une clé de chiffrement.

**[Essayer en ligne](https://nick-suebang.github.io/encodeur-et-decodeur-de-fichier/)** : le programme C++, compilé en WebAssembly, s'exécute directement dans le navigateur.

## Fonctionnalités

- Encodage et décodage de fichiers texte avec une clé
- Validation de la clé, avec une nouvelle saisie tant qu'elle est invalide
- Deux interfaces construites sur le même cœur : une application console et une page web
- Tests automatiques lancés à chaque push

## Organisation du code

```
src/chiffrement.h/.cpp   Cœur : l'algorithme seul, en fonctions pures (ni fichiers, ni console)
console/main.cpp         Application console : saisies, lecture et écriture des fichiers
web/bindings.cpp         Version web : expose le cœur à JavaScript (Emscripten)
web/index.html           Page web
tests/                   Tests du cœur
Encodeur-Decodeur.sln    Solution Visual Studio (console)
```

Le cœur ne connaît ni les fichiers ni la console : il reçoit un texte et une clé, et renvoie un résultat ou une erreur. La console et la page web ne s'occupent que des entrées et des sorties. L'algorithme n'existe donc qu'en un seul exemplaire.

## Spécifications

### Clé de chiffrement
- Longueur : entre 5 et 20 caractères
- Caractères autorisés : caractères ASCII imprimables de 33 à 126 (sans espace ni accent)

### Texte traité
- Caractères ASCII imprimables de 32 à 126
- Les retours à la ligne (`\n` et `\r`) sont conservés tels quels
- Tout autre caractère (accent, émoji, tabulation…) provoque une erreur, et aucun fichier n'est produit

## Algorithme

- Chaque caractère imprimable est décalé selon le caractère courant de la clé, qui est réutilisée en boucle.
- Le décalage se mesure par rapport à la lettre `a` : `a` décale de 0, `b` de 1, `A` de -32, etc.
- Le résultat reste toujours dans la plage 32-126 (arithmétique modulo 95).
- Le décodage applique le décalage inverse.

## Version console

### Compiler

- **Visual Studio** : ouvrir `Encodeur-Decodeur.sln`, puis Générer (C++17, sources en UTF-8).
- **g++ / clang++** :
  ```bash
  g++ -std=c++17 -I src console/main.cpp src/chiffrement.cpp -o encodeur-decodeur
  ```

### Utilisation

1. Choisir `1` pour encoder ou `2` pour décoder.
2. Entrer le nom du fichier à traiter. Il est redemandé s'il est introuvable.
3. Entrer le nom du fichier à produire. Il doit être différent du fichier à traiter.
4. Entrer la clé. Elle est redemandée tant qu'elle est invalide.

Le programme renvoie le code 0 en cas de succès et 1 en cas d'erreur. Le fichier de sortie n'est écrit que si tout le traitement a réussi.

## Version web (WebAssembly)

Site en ligne : https://nick-suebang.github.io/encodeur-et-decodeur-de-fichier/

- `web/bindings.cpp` expose les fonctions du cœur à JavaScript.
- `web/index.html` est la page : choix du mode, zone de texte ou fichier `.txt`, clé, copie et téléchargement du résultat.
- `web/build.sh` compile le tout en WebAssembly avec [Emscripten](https://emscripten.org) dans `web/dist/`.

## Tests et intégration continue

```bash
sh tests/run.sh
```

Les tests vérifient la validation de la clé, des valeurs connues, la conservation des retours à la ligne, les cas d'erreur, et 2 000 textes aléatoires. Avec une clé en minuscules, le résultat doit être identique à celui de la version d'origine ; avec n'importe quelle clé, encoder puis décoder doit redonner exactement le texte de départ.

- `.github/workflows/ci.yml` lance, à chaque push et à chaque pull request : les tests (g++), la compilation de la solution Visual Studio (MSVC, x64 et x86) et la compilation de la version web.
- `.github/workflows/pages.yml` teste, compile et publie la page sur GitHub Pages à chaque push sur `main`.

## Compatibilité avec la première version

La première version de la console ne ramenait pas le résultat dans la plage 32-126 quand la clé contenait des majuscules, des chiffres ou des symboles : elle pouvait produire des caractères de contrôle, voire un faux retour à la ligne qui rendait le décodage impossible. Ce cas est maintenant corrigé. Avec une clé en minuscules, les résultats sont identiques à ceux de la première version.
