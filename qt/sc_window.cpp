//#########################
//Pekka Kana QT Sprite Creator - sc_window.cpp
//by Danilo Lemos 2017
//#########################

#include <QFileDialog>
#include <QPixmap>
#include <QTimer>
#include <unistd.h>
#include <cstdio>
#include "sc_window.h"
#include "ui_sc_window.h"

#include "../src/pistedraw.h"
#include "../src/platform.h"
#include "../src/types.h"
#include "../src/constants.h"

AnimThread anim_thread;

void AnimThread::run(){
    active = true;
    while(active){
        if(this->window->animating)
            this->window->animate();
        QThread::usleep(16666);
    }
}

//Thread function
void SC_Window::animate(){
    int lastframe = currentframe;
    currentframe = sprite->Animoi();
    if(lastframe != currentframe){
        sprite_prototype->Piirra(0,0,currentframe);
        ui->box_frame->setValue(currentframe);
        framebox_list[sprite->sekvenssi_index]; //Change color or something
    }
}

void SC_Window::GetFrameSpinBoxes(){
    framebox_list[0] = ui->box_frame1;
    framebox_list[1] = ui->box_frame2;
    framebox_list[2] = ui->box_frame3;
    framebox_list[3] = ui->box_frame4;
    framebox_list[4] = ui->box_frame5;
    framebox_list[5] = ui->box_frame6;
    framebox_list[6] = ui->box_frame7;
    framebox_list[7] = ui->box_frame8;
    framebox_list[8] = ui->box_frame9;
    framebox_list[9] = ui->box_frameA;
}

SC_Window::SC_Window(QWidget *parent) : QMainWindow(parent), ui(new Ui::SC_Window){
    sprite_prototype = new PK2Sprite_Prototyyppi();
    sprite = new PK2Sprite(sprite_prototype,0,true,0,0);

    ui->setupUi(this);
    this->GetFrameSpinBoxes();

    this->LinkVars();

    PisteDraw2_Start(ui->frame);

    anim_thread.window = this;

    this->updateAll();
}

SC_Window::~SC_Window(){
    anim_thread.active = false;
    while(anim_thread.isRunning());

    delete ui;
    delete sprite;
    delete sprite_prototype;
}

void SC_Window::LinkVars(){
    //Menu
    connect(ui->menuOpen,    SIGNAL(triggered(bool)),         this, SLOT(open()));
    connect(ui->menuSave,    SIGNAL(triggered(bool)),         this, SLOT(save()));

    //Color
    for(int i = 0; i < 8; i++)
        ui->box_color->addItem(colorlist[i]);

    connect(ui->box_color,   SIGNAL(currentIndexChanged(int)),this, SLOT(boxcolor_changed(int)));

    //Image

    //Animation
    ui->box_curranim->link_var(sprite->animaatio_index);
    ui->box_interval->link_var(sprite_prototype->frame_rate);

    connect(ui->box_animate, SIGNAL(stateChanged(int)),       this, SLOT(boxanimate_changed(int)));
    connect(ui->box_restart, SIGNAL(clicked()),               this, SLOT(restartanimation()));
    connect(ui->box_frame,   SIGNAL(valueChanged(int)),       this, SLOT(boxcurrentframe_changed(int)));

}

//Slots
void SC_Window::open(){
    QString file = QFileDialog::getOpenFileName(this,"Selecione um sprite.", "", "Sprites (*.spr)");

    if(!file.isEmpty()){

        if(anim_thread.isRunning()){
            anim_thread.active = false;
            while(anim_thread.isRunning());
        }

        this->filename = file;
        QByteArray bap = filename.toLatin1(); //Path
        QByteArray baf = bap; //File

        bap.data()[bap.lastIndexOf(DIR_SEP)+1] = '\0';
        baf.remove(0,bap.lastIndexOf('\0'));
        chdir(bap.data());

        sprite_prototype->Lataa(bap.data(),baf.data());

        anim_thread.start();

        this->updateAll();
    }
}
void SC_Window::save(){

}
void SC_Window::reset(){

}
void SC_Window::boxcurrentframe_changed(int value){
    currentframe = value;
    sprite_prototype->Piirra(0,0,value);
}
void SC_Window::boxcurrentanimation_changed(int value){
    //sprite->animaatio_index = value;
    ui->txt_animationName->setText(animationlist[value]);
    ui->box_loop->link_var(sprite_prototype->animaatiot[value].looppi);
    this->updateFrameSpinBoxes();
}
void SC_Window::boxloop_changed(bool value){
    int currentanimation = sprite->animaatio_index;
    sprite_prototype->animaatiot[currentanimation].looppi = value;
}

void SC_Window::boxcolor_changed(int value){
    if(value != 0){
        sprite_prototype->vari = colorlist_values[value];
        sprite_prototype->UpdateColors();
        PisteDraw2_UpdateImage();
    }
    else PisteDraw2_RecoverImage();

    sprite_prototype->Piirra(0,0,currentframe);
}
void SC_Window::boxanimate_changed(int value){
    bool check = value;
    ui->box_frame->setEnabled(!check);
    ui->txt_frame->setEnabled(!check);
    animating = check;
}
void SC_Window::restartanimation(){
    sprite->sekvenssi_index = 0;
}

void SC_Window::updateFrameSpinBoxes(){
    int i;
    int currentanimation = sprite->animaatio_index;
    int nofframes = sprite_prototype->animaatiot[currentanimation].frameja;

    for(i = 0; i < nofframes; i++){
        framebox_list[i]->setEnabled(true);
        framebox_list[i]->setOffset(1);
        framebox_list[i]->link_var(sprite_prototype->animaatiot[currentanimation].sekvenssi[i]);
        framebox_list[i]->setMaximum(sprite_prototype->frameja - 1);
    }
    for(; i < 10 ; i++){
        framebox_list[i]->setEnabled(false);
        framebox_list[i]->setOffset(1);
        framebox_list[i]->link_var(sprite_prototype->animaatiot[currentanimation].sekvenssi[i]);
        framebox_list[i]->setValue(0);
    }
}

//Internal Functions
void SC_Window::updateAll(){
    this->LinkVars();

    ui->lab_sound->setText(sprite_prototype->aanitiedostot[0]);
    ui->lab_name->setText(sprite_prototype->nimi);

    int color = sprite_prototype->vari;
    ui->box_color->setEnabled(true);
    for(int i = 0; i < 8; i++)
        if(colorlist_values[i] == color)
            ui->box_color->setCurrentIndex(i);

    ui->box_frame->setMaximum(sprite_prototype->frameja - 1);
    ui->box_frame->setMinimum(0);
    if(!animating){
        ui->txt_frame->setEnabled(true);
        ui->box_frame->setEnabled(true);
        ui->box_frame->setValue(0);
        this->boxcurrentframe_changed(0);
    }

    ui->box_animate->setEnabled(true);

    ui->txt_interval->setEnabled(true);
    ui->box_interval->setEnabled(true);

    ui->txt_animationName->setEnabled(true);
    ui->txt_animationName->setText(animationlist[sprite->animaatio_index]);
    ui->txt_curranim->setEnabled(true);
    //ui->txt_nofframes->setEnabled(true);
    ui->txt_frames->setEnabled(true);

    ui->box_restart->setEnabled(true);

    int currentanimation = sprite->animaatio_index;
    ui->box_loop->setEnabled(true);
    ui->box_loop->setChecked(sprite_prototype->animaatiot[currentanimation].looppi);

    ui->box_curranim->setEnabled(true);
    ui->box_curranim->setValue(sprite->animaatio_index);
    ui->box_curranim->setMaximum(sprite_prototype->animaatioita);
    ui->box_curranim->setMinimum(0);
    this->updateFrameSpinBoxes();
}
