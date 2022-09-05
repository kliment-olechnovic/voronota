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
    T1118v1LG035_1_pose1  T1118v1LG046_1_pose2  interface                0.568301   648.036      808.139
    T1118v1LG035_1_pose1  T1118v1LG046_1_pose2  interface_plus_adjacent  0.629109   1781.9       2189.88

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
 * The 'interface' set includes only the protein-ligand contacts.
 * The 'interface_plus_adjacent' set includes the following contacts: all the 'interface' set contacts; protein-protein contacts between residues that have contacts with ligands.
 * You can use the '--details-dir' option to look at the used contacts lists.
 * You can use the '--drawing-dir' option to generate PDB files (with interface participants) and contact drawing scripts for PyMol.
 
Example pymol call for files generated using the '--drawing-dir' option:
 
    pymol \
      draw_T1118v1LG035_1_pose1_interface_and_adjacent_contacts_in_pymol.py \
      T1118v1LG035_1_pose1_cutout_interface_residues_ligand.pdb \
      T1118v1LG035_1_pose1_cutout_interface_residues_receptor.pdb

Example of visualized contacts (with direct interface contacts in green, adjacent contacts in yellow, ligand atoms in red, receptor atoms in cyan): [https://kliment-olechnovic.github.io/voronota/expansion_js/resources/texts/protein_ligand_3D.png](https://kliment-olechnovic.github.io/voronota/expansion_js/resources/texts/protein_ligand_3D.png).

