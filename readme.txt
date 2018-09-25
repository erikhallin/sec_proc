Sequence Data Processing
Made by Erik Hallin 2017

Description
This software will align and analyze the sequencing data.

Setup
1. MS Excel is required to be installed.
2. A software to extract sequencing data from .ab1 files is required. For example Chromas (http://www.technelysium.com.au/Chromas264Setup.exe).
3. Install Clustalw (http://www.clustal.org/clustal2/#Download).
   To be able to run, clustalw2.exe must be called from the command line. If not automatically added, add this file to the environment path or copy the clustalw2.exe to the active folder.
4. Either add the sec_proc.exe to the environment path to be able to call the software from the command line or add it to the active folder.

How to use
1. Extract the sequences from the .ab1 files to make one .fasta file per sequencing reaction.
2. In the active folder there should be one .xls file with the default layout. The sequencing ordering form.
   The second column contains the file identifiers and the third column has the name of the sequencing reactions.
   The names should begin with either "f_" or "r_" stating if the PCR reaction was forward or reverse.
   The end of the name should contain a number (0-9) stating that the reactions with the same name but different end number should be similar.
3. In the active folder there should be text files with the template sequences for each reaction name. One sequence per file named "template_NAME.txt".
4. Runc the sec_proc.exe in the active folder.
5. Alinments should be made for every reaction group and a analysis.txt have been made stating which reaction in each group that had the longest overlapping sequence with the template sequence.

Example:
In the sequencing.xls: 	
	EH01	f_NAME1
	EH02	f_NAME2
	EH03	r_NAME1
	EH04	r_NAME2
The file above contains 4 sequencing reactions, 2 forward and 2 reverse reactions on the same construct.	