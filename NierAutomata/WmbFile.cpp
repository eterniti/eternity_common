#include "WmbFile.h"
#include "debug.h"

WmbFile::WmbFile()
{
    this->big_endian = false;
}

WmbFile::~WmbFile()
{

}

void WmbFile::Parse(uint8_t *buf)
{
    WMBHeader *hdr = (WMBHeader *)buf;
    base = buf;

    if (hdr->signature != WMB_SIGNATURE)
    {
        DPRINTF("%s: Invalid signature.\n", FUNCNAME);
        return;
    }

    WMBMeshGroupInfo *infos = (WMBMeshGroupInfo *)(base + hdr->mesh_group_infos_offset);
    WMBMeshGroup *groups = (WMBMeshGroup *)(base + hdr->mesh_groups_offset);
    WMBMesh *meshes = (WMBMesh *)(base + hdr->meshes_offset);

    for (uint32_t i = 0; i < hdr->num_meshes; i++)
    {
        std::set<uint32_t> groups_set;

        for (uint32_t j = 0; j < hdr->num_mesh_group_infos; j++)
        {
            infos[j].GetMeshGroups(base, i, groups_set);
        }

        DPRINTF("---- Listing for %d\n", i);

        for (uint32_t group : groups_set)
        {
            if (group < hdr->num_mesh_groups)
            {
                DPRINTF("%s  %d vertex %d faces\n", groups[group].GetName(base).c_str(), meshes[i].num_vertex, meshes[i].num_faces);
            }
            else
            {
                DPRINTF("%s: Warning, group %d out of bounds.\n", FUNCNAME, group);
            }
        }

        DPRINTF("Offset: %x\n", Utils::DifPointer(&meshes[i], buf));

        DPRINTF("-----------\n");
    }
}

void WmbFile::Test()
{
    WMBHeader *hdr = (WMBHeader *)base;

    WMBMeshGroupInfo *infos = (WMBMeshGroupInfo *)(base + hdr->mesh_group_infos_offset);
    WMBMeshGroup *groups = (WMBMeshGroup *)(base + hdr->mesh_groups_offset);
    WMBMesh *meshes = (WMBMesh *)(base + hdr->meshes_offset);

    std::vector<WMBMesh> new_meshes;
    std::set<uint32_t> survivors;

    std::unordered_set<std::string> desired_groups =
    {
        /*"DLC_Body",
        "facial_normal",
        "facial_serious",
        "Hair",
        "DLC_Skirt",
        "DLC_Broken"*/

        "facial_normal",
        "facial_serious",
        //"Hair",
        "Body",
        "Skirt",
        "eyelash",
        "feather",
        "DLC_Body",
        "DLC_Skirt",
        "Eyemask"

        //"Hair"
    };

    /*std::unordered_set<uint32_t> ban_list =
    {
        -1
    };*/

    for (uint32_t i = 0; i < hdr->num_meshes; i++)
    {
        std::set<uint32_t> groups_set;

        for (uint32_t j = 0; j < hdr->num_mesh_group_infos; j++)
        {
            infos[j].GetMeshGroups(base, i, groups_set);
        }

        for (uint32_t group : groups_set)
        {
            if (group < hdr->num_mesh_groups)
            {
                std::string name = groups[group].GetName(base);

                if (desired_groups.find(name) != desired_groups.end())
                {
                    survivors.insert(i);
                }
            }
            else
            {
                DPRINTF("%s: Warning, group %d out of bounds.\n", FUNCNAME, group);
            }
        }

        //DPRINTF("-----------\n");
    }

    /*DPRINTF("Survivors list: ");
    for (uint32_t mesh : survivors)
    {
        DPRINTF("%d ", mesh);
    }
    DPRINTF("\n");*/

    for (size_t i = 0; i < hdr->num_meshes; i++)
    {
        if (survivors.find((uint32_t)i) != survivors.end() /*&& ban_list.find((uint32_t)i) == ban_list.end()*/)
        {
            //new_meshes.push_back(meshes[i]);
        }
        else
        {
            meshes[i].num_faces = meshes[i].num_vertex = 0;
        }
    }

    //memcpy(meshes, new_meshes.data(), new_meshes.size()*sizeof(WMBMesh));
    //hdr->num_meshes = (uint32_t)new_meshes.size();
}

void WMBMeshGroupInfo::GetMeshGroups(uint8_t *base, uint32_t mesh, std::set<uint32_t> &groups)
{
    if (mesh < mesh_start || mesh >= (mesh_start+num_meshes))
        return;

    //DPRINTF("Range [%d-%d]\n", mesh_start, mesh_start+num_meshes-1);

    WMBGroupedMesh *grouped = ((WMBGroupedMesh *)(base + grouped_mesh_start)) + (mesh - mesh_start);
    groups.insert(grouped->mesh_group_index);
}


