#! /usr/bin/python

import errno
import os
import multiprocessing as mp
import ROOT
import sys


def get_final_states(infile):
    """
    Return a list of strings that correspond to final states in the ROOT file.  The strings are retrieved from the names of the histogram TDirectories in the file.
    """
    tfile = ROOT.TFile(infile,"READ")
    final_states = []
    for key in tfile.GetListOfKeys():
        obj = key.ReadObj()
        if obj.ClassName() == "TDirectoryFile":
            final_states.append("_".join(obj.GetName().split("_")[1:]))
    return final_states


def process_final_state(final_state,outfilename,rootfiles,tree_names):
    print(outfilename+" "+final_state)
    try:
        os.mkdir(final_state)
    except OSError as e:
        if e.errno == errno.EEXIST:
            pass
    outfile = ROOT.TFile(final_state+"/"+outfilename,"RECREATE")
    for tree_name in tree_names:
        if tree_name.split("___")[0] == final_state:
            newname = "___".join(tree_name.split("___")[1:])
            tchain = ROOT.TChain(tree_name)
            for rootfile in rootfiles:
                tchain.Add(rootfile)
            outtree = tchain.CloneTree(-1,"fast")
            if newname == "":
                newname = "NOSYS"
            try:
                outtree.SetName(newname)
            except ReferenceError:
                continue
            outtree.Write()
    outfile.Close()

def process_final_state_(final_state,outfilename,rootfiles,tree_names):
    print(outfilename+" "+final_state)

def get_tree_names(infile):
    """
    Return a list of strings corresponding to the name of every TTree in a ROOT file.
    """
    tfile = ROOT.TFile(infile)
    tree_names = []
    for key in tfile.GetListOfKeys():
        obj = key.ReadObj()
        if obj.ClassName() == "TTree":
            tree_names.append(obj.GetName())
    return tree_names


if __name__ == "__main__":
    if(sys.argv[2]=='1'):
        ROOT.TTree.SetMaxTreeSize(10*ROOT.TTree.GetMaxTreeSize())
        sys.argv[1] = sys.argv[1].rstrip("/")
        outfilename = sys.argv[1].split("/")[-1]
        if outfilename.split(".")[-1] != "root":
            outfilename += ".root"
        rootfiles = []
        if os.path.isdir(sys.argv[1]):
            for root,_,files in os.walk(sys.argv[1]):
                for filename in files:
                    if filename.split(".")[-1] == "root":
                        rootfiles.append(root+"/"+filename)
        else:
            rootfiles.append(sys.argv[1])

        final_states = get_final_states(rootfiles[0])
        tree_names = get_tree_names(rootfiles[0])

        processes = [ mp.Process(target=process_final_state, args=(final_state,outfilename,rootfiles,tree_names)) for final_state in final_states ]
        for process in processes:
            process.start()
        for process in processes:
            process.join()
    
    if(sys.argv[2]=='0' and sys.argv[3]=='0'):
        ROOT.TTree.SetMaxTreeSize(10*ROOT.TTree.GetMaxTreeSize())
        sys.argv[1] = sys.argv[1].rstrip("/")
        outfilename = sys.argv[1].split("/")[-1]
        if outfilename.split(".")[-1] != "root":
            outfilename += ".root"
        rootfiles = []
        if os.path.isdir(sys.argv[1]):
            for root,_,files in os.walk(sys.argv[1]):
                for filename in files:
                    if filename.split(".")[-1] == "root":
                        rootfiles.append(root+"/"+filename)
        else:
            rootfiles.append(sys.argv[1])

        final_states = get_final_states(rootfiles[0])
        top = '/raid05/users/muse/ts/'
        for bo in final_states:
            os.system('cp -rf '+top+bo+' .')
        tree_names = get_tree_names(rootfiles[0])

        processes = [ mp.Process(target=process_final_state_, args=(final_state,outfilename,rootfiles,tree_names)) for final_state in final_states ]
        for process in processes:
            process.start()
        for process in processes:
            process.join()
    if(sys.argv[2]=='0' and sys.argv[3]>'0'):
        ROOT.TTree.SetMaxTreeSize(10*ROOT.TTree.GetMaxTreeSize())
        sys.argv[1] = sys.argv[1].rstrip("/")
        outfilename = sys.argv[1].split("/")[-1]
        if outfilename.split(".")[-1] != "root":
            outfilename += ".root"
        rootfiles = []
        if os.path.isdir(sys.argv[1]):
            for root,_,files in os.walk(sys.argv[1]):
                for filename in files:
                    if filename.split(".")[-1] == "root":
                        rootfiles.append(root+"/"+filename)
        else:
            rootfiles.append(sys.argv[1])

        final_states = get_final_states(rootfiles[0])
        tree_names = get_tree_names(rootfiles[0])

        processes = [ mp.Process(target=process_final_state_, args=(final_state,outfilename,rootfiles,tree_names)) for final_state in final_states ]
        for process in processes:
            process.start()
        for process in processes:
            process.join()
