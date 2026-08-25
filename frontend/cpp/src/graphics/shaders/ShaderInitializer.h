//
// Created by tim on 17.07.26.
//

#ifndef HARMONY_SHADERINITIALIZER_H
#define HARMONY_SHADERINITIALIZER_H
#include "../widgets/FilledButton.h"
#include "../widgets/TextField.h"

void initializeShaders() {
    FilledButton::initializeShaders();
    TextField::initializeShaders();
}

void deleteShaders() {
    FilledButton::deleteShaders();
    TextField::deleteShaders();
}

#endif //HARMONY_SHADERINITIALIZER_H
