import bpy
import os
import bmesh
from subprocess import call
basedir = os.path.dirname(bpy.data.filepath)
if not basedir:
    raise Exception("Blend file is not saved")
scene = bpy.context.scene
selection = bpy.context.selected_objects
for obj in selection:
    obj.select = False
for obj in selection:
    name = bpy.path.clean_name(obj.name)
    fn = os.path.join(basedir, name) + ".mesh"
    out = open( fn , "w" )
    out.write( obj.type + ": " + obj.name + "\n" )
    tmp_obj = obj.copy()#bpy.data.objects.new(name + "temp", bpy.data.meshes.new(name + "temp"))
    tmp_obj.data = obj.data.copy()
    #mesh.scale = obj.scale
    #mesh.location = obj.location
    #mesh = tmp_obj.data
    bm = bmesh.new()
    bm.from_mesh(tmp_obj.data)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    bm.to_mesh(tmp_obj.data)
    bm.free()
    mesh = tmp_obj.data
    out.write( "POSITION\n" )
    for vert in mesh.vertices:
        out.write( 'v %f %f %f\n' % (vert.co.x, vert.co.y, vert.co.z) )
    out.write( "NORMAL\n" )
    for vert in mesh.vertices:
        out.write( 'n %f %f %f\n' % (vert.normal.x, vert.normal.y, vert.normal.z) )
    for uv_layer in mesh.uv_layers:    
        out.write( "TEXCOORD\n" )
        for face in mesh.polygons:
            for i in face.loop_indices:
                uvCoord = uv_layer.data[i].uv
                out.write( 'tx %f %f' % (uvCoord.x , uvCoord.y) )
                out.write('\n')
    out.write( "FACE\n" )
    for face in mesh.polygons:
        out.write( 'f' ) 
        for index in face.vertices:
            out.write( ' %i' % index )
        out.write('\n')
    out.write( "TEXTURE_FACE\n" )
    for face in mesh.polygons:
        out.write( 'f' ) 
        for i in face.loop_indices:
            out.write( ' %i' % i )
        out.write('\n')
     
    """if mesh.materials is not None and mesh.materials[ 0 ] is not None and mesh.materials[ 0 ].active_texture is not None:
        out.write( "TEXTURE\n" )
        out.write( mesh.materials[ 0 ].active_texture.image.filepath )
        out.write('\n')"""
    out.write( "END" )
    out.close()
    tmp_obj.select = True
    bpy.ops.object.delete() 
    print( "MeshParser" + " " + name + ".mesh" )
    call( [ "./MeshParser" , "./" + name + ".mesh" ] )
