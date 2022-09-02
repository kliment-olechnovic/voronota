# Protein-ligand variation of CAD-score

## Availability

The script 'voronota-js-ligand-cadscore' for the protein-ligand variation of CAD-score is included to the Voronota-JS package available from [https://kliment-olechnovic.github.io/voronota/expansion_js/](https://kliment-olechnovic.github.io/voronota/expansion_js/).

### Script interface
    
    Input options, basic:
        --target-receptor             string  *  target receptor file path
        --target-ligands              string  *  list of target ligand file paths
        --target-ligand-ids           string  *  list of target ligand IDs
        --model-receptor              string  *  model receptor file path
        --model-ligands               string  *  list of model ligand file paths
        --model-ligand-ids            string  *  list of model ligand IDs
        --target-name                 string     target to use for output, default is 'target_complex'
        --model-name                  string     model name to use for output, default is 'model_complex'
    
    Input options, alternative:
        --casp15-target               string  *  target data file in CASP15 format, alternative to --target-* options
        --casp15-target-pose          string  *  pose number to select from the target data file in CASP15 format
        --casp15-model                string  *  model data file in CASP15 format, alternative to --model-* options
        --casp15-model-pose           string  *  pose number to select from the model data file in CASP15 format
    
    Other options:
        --details-dir                 string     directory to output lists of contacts used for scoring
        --drawing-dir                 string     directory to output files to visualize with pymol
        --and-swap                    string     flag to compute everything after swapping target and model, default is 'false'
        --help | -h                              display help message and exit
        
    Standard output:
        space-separated table of scores
        
    Examples:
    
        voronota-js-ligand-cadscore --casp15-target ./T1118v1LG035_1 --casp15-target-pose 1 --casp15-model ./T1118v1LG046_1 --casp15-model-pose 1
        
        voronota-js-ligand-cadscore \
          --target-receptor ./t_protein.pdb --target-ligands './t_ligand1.mol ./t_ligand2.mol ./t_ligand3.mol' --target-ligand-ids 'a a b' \
          --model-receptor ./m_protein.pdb --model-ligands './m_ligand1.mol ./m_ligand2.mol ./m_ligand3.mol' --model-ligand-ids 'a a b'


## Example of how it works for CASP15 models

Let us take two files with protein-ligand models from CASP15: 'H1135LG035_1' and 'H1135LG046_1'.
Then let us execute the following command:

    voronota-js-ligand-cadscore \
      --casp15-target /home/kliment/casp15/data/ligands/T1118v1/T1118v1LG035_1 \
      --casp15-model /home/kliment/casp15/data/ligands/T1118v1/T1118v1LG046_1 \
      --casp15-target-pose 1 \
      --casp15-model-pose 2 \
    | column -t

which outputs:

    target                model                 contacts_set             CAD_score  target_area  model_area
    T1118v1LG035_1_pose1  T1118v1LG046_1_pose2  interface                0.606392   714.927      876.674
    T1118v1LG035_1_pose1  T1118v1LG046_1_pose2  interface_plus_adjacent  0.641639   1848.79      2258.41

### Output explanation

The output table columns mean the following

 * 'target' - name of the target protein-ligand complex structure.
 * 'model' - name of the model protein-ligand complex structure.
 * 'contacts_set' - indicates what contacts were compared: only the direct interface contacts ('interface'), or the expanded set that also contains the contacts adjacent to the interface ('interface_plus_adjacent').
 * 'CAD_score' - CAD-score value computed using the standard CAD-score formula (the formula can be seen on [https://bioinformatics.lt/cad-score/theory](https://bioinformatics.lt/cad-score/theory)).
 * 'target_area' - total area of the relevant contacts in the target complex
 * 'model_area' - total area of the relevant contacts in the model complex

## More info about the involved contacts

There are some important aspects:

 * All the contact areas are computed on the atom-atom level, but then bundled (summed) to the residue-residue/residue-ligand level.
 * When defining the contact uniqueness, residue names and numbers (and chain ) are used for the protein side, but only ligand names are used for the ligand side. Therefore, the script treats same-named ligands as a single ligand when summarizing contact areas. This was done to avoid doing permutations of ligand numbers for identical ligands.
 * The 'interface' set includes the following contacts: protein-ligand contacts and ligand-ligand contacts.
 * The 'interface_plus_adjacent' set includes the following contacts: all the 'interface' set contacts; protein-protein contacts between residues that have contacts with ligands.
 * You can use the '--details-dir' option to look at the used contacts lists.
 * You can use the '--drawing-dir' option to generate PDB files (with interface participants) and contact drawing scripts for PyMol.
 
Example pymol call for files generated using the '--drawing-dir' option:
 
    pymol \
      draw_T1118v1LG035_1_pose1_interface_and_adjacent_contacts_in_pymol.py \
      T1118v1LG035_1_pose1_cutout_interface_residues_ligand.pdb \
      T1118v1LG035_1_pose1_cutout_interface_residues_receptor.pdb

Example of visualized contacts (with direct interface contacts in green, adjacent contacts in yellow, ligand atoms in red, receptor atoms in cyan): [https://kliment-olechnovic.github.io/voronota/expansion_js/resources/texts/protein_ligand_3D.png](https://kliment-olechnovic.github.io/voronota/expansion_js/resources/texts/protein_ligand_3D.png).

