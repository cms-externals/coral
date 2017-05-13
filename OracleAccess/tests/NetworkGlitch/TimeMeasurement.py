#!/usr/bin/env python
import os, sys, unittest
from ssh_tunnel import StoneHandler
from time import sleep
from time import time

import commands
import sys, os, string, fileinput
import ROOT
from ROOT import TCanvas,TH1F,gROOT,TFile,gDirectory,gStyle

ROOT.gROOT.SetBatch(ROOT.kFALSE)

rootfile = TFile.Open("TimeMeasurement_v001.root","RECREATE");

h1 = TH1F("Histo1","Histo1",100, 0., 500)
h2 = TH1F("Histo2","Histo2",100, 0., 2)
h3 = TH1F("Histo3","Histo3",100, 0., 5)
h4 = TH1F("Histo4","Histo4",100, 0., 2)
h5 = TH1F("Histo5","Histo5",100, 0., 10)
h6 = TH1F("Histo6","Histo6",100, 0., 25)

histolist = []
histolist.append(h1)
histolist.append(h2)
histolist.append(h3)
histolist.append(h4)
histolist.append(h5)
histolist.append(h6)

hc1 = TH1F("hConnect1","hConnect1",100, 0., 100)
hc2 = TH1F("hConnect2","hConnect2",100, 0., 0.1)
hc3 = TH1F("hConnect3","hConnect3",100, 0., 0.1)
hc4 = TH1F("hConnect4","hConnect4",100, 0., 100)
hc5 = TH1F("hConnect5","hConnect5",100, 0., 20)
hc6 = TH1F("hConnect6","hConnect6",100, 0., 0.5)

histolistc = []
histolistc.append(hc1)
histolistc.append(hc2)
histolistc.append(hc3)
histolistc.append(hc4)
histolistc.append(hc5)
histolistc.append(hc6)

hs1 = TH1F("hSession1","hSession1",100, 0., 100)
hs2 = TH1F("hSession2","hSession2",100, 0., 0.1)
hs3 = TH1F("hSession3","hSession3",100, 0., 0.1)
hs4 = TH1F("hSession4","hSession4",100, 0., 100)
hs5 = TH1F("hSession5","hSession5",100, 0., 20)
hs6 = TH1F("hSession6","hSession6",100, 0., 0.5)

histolists = []
histolists.append(hs1)
histolists.append(hs2)
histolists.append(hs3)
histolists.append(hs4)
histolists.append(hs5)
histolists.append(hs6)

for i in range(10000):
    os.system("./PyCoralMultiTest_000.py")
    #os.system("mv test_000.txt TimeMeasurementDir/test_000_"+str(i)+".txt")
    in_file = open("TimeMeasurementDir/test_000_"+str(i)+".txt","r")

    counter = 0
    for line in in_file:
        histolist[counter].Fill(1000*float((line.rstrip().split(":"))[1]))
        counter += 1

    #os.system("mv time_connect_000.txt TimeMeasurementDir/time_connect_000_"+str(i)+".txt")
    in_file = open("TimeMeasurementDir/time_connect_000_"+str(i)+".txt","r")

    for line in in_file:
        listvalue = line.rstrip().split("  ")
        counter = 0
        for el in listvalue:
            histolistc[counter].Fill(float(listvalue[counter]))
            counter += 1

    os.system("mv time_session_000.txt TimeMeasurementDir/time_session_000_"+str(i)+".txt")
    in_file = open("TimeMeasurementDir/time_session_000_"+str(i)+".txt","r")

    for line in in_file:
        listvalue = line.rstrip().split("  ")
        counter = 0
        for el in listvalue:
            histolists[counter].Fill(float(listvalue[counter]))
            counter += 1
            
        
rootfile.Write()



