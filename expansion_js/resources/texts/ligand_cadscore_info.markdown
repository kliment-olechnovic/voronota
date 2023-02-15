# More about protein-ligand variation of CAD-score

The script 'voronota-js-ligand-cadscore' for the protein-ligand variation of CAD-score
is included to [the Voronota-JS expansion](./index.html) of Voronota.

## Example of how it works for CASP15 models

The 'voronota-js-ligand-cadscore' scripts has a special input mode for CASP15 protein-ligand model files: when using --casp15-* options for specifying input, the input files are split into receptor and ligand parts and passed to the same script with basic input options. The target input in the CASP15 mode should be formatted in the same as the model input. The basic input mode option is more flexible, but it requires all the receptor and ligand files available separately, and the ligand IDs (names) to be specified explicitly.

For example, let us take two files with protein-ligand models from CASP15: 'H1135LG035_1' and 'H1135LG046_1'.
Then let us execute the following command:

    voronota-js-ligand-cadscore \
      --casp15-target ./T1118v1LG035_1 \
      --casp15-model ./T1118v1LG046_1 \
      --casp15-target-pose 1 \
      --casp15-model-pose 2 \
    | column -t

which outputs:

    target                model                 contacts_set             CAD_score  target_area  model_area
    T1118v1LG035_1_pose1  T1118v1LG046_1_pose2  interface                0.568301   648.036      808.139
    T1118v1LG035_1_pose1  T1118v1LG046_1_pose2  interface_plus_adjacent  0.629109   1781.9       2189.88

Note, that CAD-score values will be different if we swap target and model inputs. The script can do the swap automatically and output additional results when the '--and-swap true' is added to the arguments, then the output will look as follows:

    target                model                 contacts_set             CAD_score  target_area  model_area
    T1118v1LG035_1_pose1  T1118v1LG046_1_pose2  interface                0.568301   648.036      808.139
    T1118v1LG035_1_pose1  T1118v1LG046_1_pose2  interface_plus_adjacent  0.629109   1781.9       2189.88
    T1118v1LG046_1_pose2  T1118v1LG035_1_pose1  interface                0.528076   808.139      648.036
    T1118v1LG046_1_pose2  T1118v1LG035_1_pose1  interface_plus_adjacent  0.519405   2189.88      1781.9

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
 * When defining the contact uniqueness, residue names and numbers (and chain ID, if it is present) are used for the protein side, but only ligand names are used for the ligand side. Therefore, the script treats same-named ligands as a single ligand when summarizing contact areas. This was done to avoid doing permutations of ligand numbers for identical ligands.
 * The 'interface' set includes only the protein-ligand contacts.
 * The 'interface_plus_adjacent' set includes the following contacts: all the 'interface' set contacts; protein-protein contacts between residues that have contacts with ligands.
 * You can use the '--details-dir' option to look at the used contacts lists.
 * You can use the '--drawing-dir' option to generate PDB files (with interface participants) and contact drawing scripts for PyMol.

### Details example

Examples of files generated using the '--details-dir' option:

 * 'interface' contacts:
     * [T1118v1LG035_1_pose1_interface_contacts.txt](./tests/jobs_output/ligand_cadscore_script/details/T1118v1LG035_1_pose1_interface_contacts.txt)
     * [T1118v1LG046_1_pose2_interface_contacts.txt](./tests/jobs_output/ligand_cadscore_script/details/T1118v1LG046_1_pose2_interface_contacts.txt)
 * 'interface_plus_adjacent' contacts:
     * [T1118v1LG035_1_pose1_interface_and_adjacent_contacts.txt](./tests/jobs_output/ligand_cadscore_script/details/T1118v1LG035_1_pose1_interface_and_adjacent_contacts.txt)
     * [T1118v1LG046_1_pose2_interface_and_adjacent_contacts.txt](./tests/jobs_output/ligand_cadscore_script/details/T1118v1LG046_1_pose2_interface_and_adjacent_contacts.txt)

### Drawing example

Example pymol call for displaying files generated using the '--drawing-dir' option:
 
    pymol \
      draw_T1118v1LG035_1_pose1_interface_and_adjacent_contacts_in_pymol.py \
      T1118v1LG035_1_pose1_cutout_interface_residues_ligand.pdb \
      T1118v1LG035_1_pose1_cutout_interface_residues_receptor.pdb

Example of visualized contacts (with direct interface contacts in green, adjacent contacts in yellow, ligand atoms in red, receptor atoms in cyan): [protein_ligand_3D.png](./resources/texts/protein_ligand_3D.png).

