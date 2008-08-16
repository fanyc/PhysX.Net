#include "StdAfx.h"

#include "Core.h"
#include "Core Description.h"
#include "User Output Stream.h"
#include "Scene Description.h"
#include "Scene.h"
#include "Convex Mesh.h"
#include "Convex Mesh Description.h"
#include "Height Field Description.h"
#include "CCD Skeleton.h"
#include "Simple Triangle Mesh.h"
#include "Foundation.h"
#include "Memory Reader Stream.h"

using namespace StillDesign::PhysX;

Core::Core()
{
	CreateCore( gcnew CoreDescription(), nullptr );
}
Core::Core( CoreDescription^ description, StillDesign::PhysX::UserOutputStream^ userOutputStream )
{
	CreateCore( description, userOutputStream );
}
Core::Core( NxPhysicsSDK* core )
{
	Debug::Assert( core != NULL );
	
	ObjectCache::Add( (intptr_t)core, this );
	
	_physicsSDK = core;
	
	for( unsigned int x = 0; x < core->getNbScenes(); x++ )
	{
		Scene^ scene = gcnew Scene( core->getScene( x ) );
		
		_sceneCollection->Add( scene );
	}
	//for( int x = 0; x < sdk->getNbTriangleMeshes(); x++ )
	//{
	//	TriangleMesh^ triangleMesh = TriangleMesh::FromUnmanagedPointer( this, sdk->get
	//}
	for( unsigned int x = 0; x < core->getNbCCDSkeletons(); x++ )
	{
		//CCDSkeleton* skeleton = CCDSkeleton::FromUnmanaged( sdk->getccd
	}
	
	CreateAux();
}
Core::~Core()
{
	this->!Core();
}
Core::!Core()
{
	if( this->IsDisposed == true )
		return;
	
	onDisposing( this, nullptr );
	
	// Delete Children
	_sceneCollection->DiposeOfAll();
	_triangleMeshCollection->DiposeOfAll();
	_convexMeshCollection->DiposeOfAll();
	_clothMeshCollection->DiposeOfAll();
	_heightFieldCollection->DiposeOfAll();
	_CCDSkeletonCollection->DiposeOfAll();
	_softBodyMeshCollection->DiposeOfAll();
	
	if( _physicsSDK != NULL )
	{
		NxReleasePhysicsSDK( _physicsSDK );
		
		_physicsSDK = NULL;
	}
	
	_sceneCollection = nullptr;
	_triangleMeshCollection = nullptr;
	_convexMeshCollection = nullptr;
	_clothMeshCollection = nullptr;
	_heightFieldCollection = nullptr;
	_CCDSkeletonCollection = nullptr;
	
	_userOutputStream = nullptr;
	
	_foundation = nullptr;
	
	onDisposed( this, nullptr );
}

bool Core::IsDisposed::get()
{
	return ( _physicsSDK == NULL );
}

void Core::CreateCore( CoreDescription^ desc, StillDesign::PhysX::UserOutputStream^ userOutputStream )
{
	_userOutputStream = userOutputStream;
	
	NxSDKCreateError error;
	NxUserOutputStream* out = ( userOutputStream == nullptr ? NULL : userOutputStream->UnmanagedPointer );
	
	_physicsSDK = NxCreatePhysicsSDK( NX_PHYSICS_SDK_VERSION, NULL, out, *desc->UnmanagedPointer, &error );
	
	if( _physicsSDK == NULL || error != 0 )
		throw gcnew Exception( "PhysX failed to initialize", gcnew Exception( String::Format( "Error code: {0} ({1})", (int)error, (CoreCreationError)error ) ) );
	
	ObjectCache::Add( (intptr_t)_physicsSDK, this );
	CreateAux();
}
void Core::CreateAux()
{
	_sceneCollection = gcnew ElementCollection< Scene^, SceneCollection^ >();
	_triangleMeshCollection = gcnew ElementCollection< TriangleMesh^, TriangleMeshCollection^ >();
	_convexMeshCollection = gcnew ElementCollection< ConvexMesh^, ConvexMeshCollection^ >();
	_clothMeshCollection = gcnew ElementCollection< ClothMesh^, ClothMeshCollection^ >();
	_heightFieldCollection = gcnew ElementCollection< HeightField^, HeightFieldCollection^ >();
	_CCDSkeletonCollection = gcnew ElementCollection< CCDSkeleton^, CCDSkeletonCollection^ >();
	_softBodyMeshCollection = gcnew ElementCollection< SoftBodyMesh^, SoftBodyMeshCollection^ >();
	
	_foundation = gcnew StillDesign::PhysX::Foundation( &_physicsSDK->getFoundationSDK() );
}

Scene^ Core::AddScene( NxScene* scene )
{
	Scene^ s = gcnew Scene( scene );
	
	_sceneCollection->Add( s );
	
	return s;
}
TriangleMesh^ Core::AddTriangleMesh( NxTriangleMesh* triangleMesh )
{
	TriangleMesh^ t = gcnew TriangleMesh( triangleMesh, this );
	
	_triangleMeshCollection->Add( t );
	
	return t;
}
ConvexMesh^ Core::AddConvexMesh( NxConvexMesh* convexMesh )
{
	ConvexMesh^ c = gcnew ConvexMesh( convexMesh, this );
	
	_convexMeshCollection->Add( c );
	
	return c;
}
ClothMesh^ Core::AddClothMesh( NxClothMesh* clothMesh )
{
	ClothMesh^ c = gcnew ClothMesh( clothMesh, this );
	
	_clothMeshCollection->Add( c );
	
	return c;
}
HeightField^ Core::AddHeightField( NxHeightField* heightField )
{
	HeightField^ h = gcnew HeightField( heightField, this );
	
	_heightFieldCollection->Add( h );
	
	return h;
}
CCDSkeleton^ Core::AddCCDSkeleton( NxCCDSkeleton* skeleton )
{
	CCDSkeleton^ s = gcnew CCDSkeleton( skeleton, this );
	
	_CCDSkeletonCollection->Add( s );
	
	return s;
}
SoftBodyMesh^ Core::AddSoftBodyMesh( NxSoftBodyMesh* mesh )
{
	SoftBodyMesh^ s = gcnew SoftBodyMesh( mesh, this );
	
	_softBodyMeshCollection->Add( s );
	
	return s;
}

//

Scene^ Core::CreateScene()
{
	return CreateScene( Vector3( 0.0f, -9.81f, 0.0f ), true );
}
Scene^ Core::CreateScene( Vector3 gravity, bool groundPlane )
{
	SceneDescription^ description = gcnew SceneDescription();
		description->Gravity = gravity;
		description->GroundPlaneEnabled = groundPlane;
	
	return CreateScene( description );
}
Scene^ Core::CreateScene( SceneDescription^ sceneDescription )
{
	if( sceneDescription == nullptr )
		throw gcnew ArgumentNullException( "sceneDescription" );
	
	NxSceneDesc* sceneDesc = sceneDescription->UnmanagedPointer;
	
	const NxSceneDesc desc = *sceneDesc;
	NxScene* scene = _physicsSDK->createScene( desc );
	
	Scene^ s = gcnew Scene( scene );
		s->UserNotify = sceneDescription->UserNotify;
		s->FluidUserNotify = sceneDescription->FluidUserNotify;
		s->UserContactModify = sceneDescription->UserContactModify;
		s->UserContactReport = sceneDescription->UserContactReport;
		s->UserTriggerReport = sceneDescription->UserTriggerReport;
		s->Name = sceneDescription->Name;
		s->UserData = sceneDescription->UserData;
	
	_sceneCollection->Add( s );
	
	return s;
}

TriangleMesh^ Core::CreateTriangleMesh( Stream^ stream )
{
	if( stream == nullptr )
		throw gcnew ArgumentNullException( "stream" );
	if( stream->CanRead == false )
		throw gcnew ArgumentException( "Cannot read from stream" );
	if( stream->Length == 0 )
		throw gcnew ArgumentException( "Stream is of length 0" );
	
	array<Byte>^ data = gcnew array<Byte>( (int)stream->Length );
	stream->Read( data, 0, (int)stream->Length );
	
	MemoryReaderStream^ reader = gcnew MemoryReaderStream( data );
	
	NxTriangleMesh* triangleMesh = _physicsSDK->createTriangleMesh( *reader->UnmanagedPointer );
	if( triangleMesh == NULL )
		throw gcnew Exception( "Unable to create triangle mesh" );
	
	TriangleMesh^ tm = gcnew TriangleMesh( triangleMesh, this );
	
	_triangleMeshCollection->Add( tm );
	
	return tm;
}
ConvexMesh^ Core::CreateConvexMesh( Stream^ stream )
{
	if( stream == nullptr )
		throw gcnew ArgumentNullException( "stream" );
	if( stream->CanRead == false )
		throw gcnew ArgumentException( "Cannot read from stream" );
	if( stream->Length == 0 )
		throw gcnew ArgumentException( "Stream is of length 0" );
	
	array<Byte>^ data = gcnew array<Byte>( (int)stream->Length );
	stream->Read( data, 0, (int)stream->Length );
	
	MemoryReaderStream^ reader = gcnew MemoryReaderStream( data );
	
	NxConvexMesh* convexMesh = _physicsSDK->createConvexMesh( *reader->UnmanagedPointer );
	if( convexMesh == NULL )
		throw gcnew Exception( "Unable to create convexMesh" );
	
	StillDesign::PhysX::ConvexMesh^ c = gcnew StillDesign::PhysX::ConvexMesh( convexMesh, this );
	
	_convexMeshCollection->Add( c );
	
	return c;
}
ClothMesh^ Core::CreateClothMesh( Stream^ stream )
{
	if( stream == nullptr )
		throw gcnew ArgumentNullException( "stream" );
	if( stream->CanRead == false )
		throw gcnew ArgumentException( "Cannot read from stream" );
	if( stream->Length == 0 )
		throw gcnew ArgumentException( "Stream is of length 0" );
	
	array<Byte>^ data = gcnew array<Byte>( (int)stream->Length );
	stream->Read( data, 0, (int)stream->Length );
	
	MemoryReaderStream^ reader = gcnew MemoryReaderStream( data );
		
	NxClothMesh* clothMesh = _physicsSDK->createClothMesh( *reader->UnmanagedPointer );
	if( clothMesh == null )
		throw gcnew Exception( "Cloth mesh failed to be created" );
	
	ClothMesh^ cm = gcnew ClothMesh( clothMesh, this );
	
	_clothMeshCollection->Add( cm );
	
	return cm;
}
HeightField^ Core::CreateHeightField( HeightFieldDescription^ description )
{
	if( description == nullptr )
		throw gcnew ArgumentNullException( "description" );
	
	NxHeightField* hf = _physicsSDK->createHeightField( *description->UnmanagedPointer );
	if( hf == null )
		throw gcnew Exception( "Height field failed to be created" );
	
	HeightField^ heightField = gcnew HeightField(  hf, this );
	
	_heightFieldCollection->Add( heightField );
	
	return heightField;
}
CCDSkeleton^ Core::CreateCCDSkeleton( SimpleTriangleMesh^ simpleTriangleMesh )
{
	ThrowIfNullOrDisposed( simpleTriangleMesh, "simpleTriangleMesh" );
	
	NxCCDSkeleton* s = _physicsSDK->createCCDSkeleton( *simpleTriangleMesh->UnmanagedPointer );
	
	if( s == NULL )
		throw gcnew Exception( "Failed to create CCD skeleton" );
	
	CCDSkeleton^ skeleton = gcnew CCDSkeleton( s, this );
	
	_CCDSkeletonCollection->Add( skeleton );
	
	return skeleton;
}
SoftBodyMesh^ Core::CreateSoftBodyMesh( Stream^ stream )
{
	if( stream == nullptr )
		throw gcnew ArgumentNullException( "stream" );
	if( stream->CanRead == false )
		throw gcnew ArgumentException( "Cannot read from stream" );
	if( stream->Length == 0 )
		throw gcnew ArgumentException( "Stream is of length 0" );
	
	array<Byte>^ data = gcnew array<Byte>( (int)stream->Length );
	stream->Read( data, 0, (int)stream->Length );
	
	MemoryReaderStream^ reader = gcnew MemoryReaderStream( data );
	
	NxSoftBodyMesh* softBodyMesh = _physicsSDK->createSoftBodyMesh( *reader->UnmanagedPointer );
	if( softBodyMesh == NULL )
		throw gcnew Exception( "Failed to create soft body mesh" );
	
	SoftBodyMesh^ s = gcnew SoftBodyMesh( softBodyMesh, this );
	
	_softBodyMeshCollection->Add( s );
	
	return s;
}

float Core::GetParameter( PhysicsParameter parameter )
{
	return _physicsSDK->getParameter( (NxParameter)parameter );
}
void Core::SetParameter( PhysicsParameter parameter, float value )
{
	_physicsSDK->setParameter( (NxParameter)parameter, value );
}
void Core::SetParameter( PhysicsParameter parameter, bool enabled )
{
	_physicsSDK->setParameter( (NxParameter)parameter, enabled ? 1.0f : 0.0f );
}

Core::SceneCollection^ Core::Scenes::get()
{
	return _sceneCollection->ReadOnlyCollection;
}
Core::TriangleMeshCollection^ Core::TriangleMeshes::get()
{
	return _triangleMeshCollection->ReadOnlyCollection;
}
Core::ConvexMeshCollection^ Core::ConvexMeshes::get()
{
	return _convexMeshCollection->ReadOnlyCollection;
}
Core::ClothMeshCollection^ Core::ClothMeshes::get()
{
	return _clothMeshCollection->ReadOnlyCollection;
}
Core::HeightFieldCollection^ Core::HeightFields::get()
{
	return _heightFieldCollection->ReadOnlyCollection;
}
Core::CCDSkeletonCollection^ Core::CCDSkeletons::get()
{
	return _CCDSkeletonCollection->ReadOnlyCollection;
}
Core::SoftBodyMeshCollection^ Core::SoftBodyMeshes::get()
{
	return _softBodyMeshCollection->ReadOnlyCollection;
}

StillDesign::PhysX::Foundation^ Core::Foundation::get()
{
	return _foundation;
}


int Core::NumberOfPhysicsProcessingUnits::get()
{
	return _physicsSDK->getNbPPUs();
}

StillDesign::PhysX::HardwareVersion Core::HardwareVersion::get()
{
	return (StillDesign::PhysX::HardwareVersion)_physicsSDK->getHWVersion();
}
Version^ Core::InternalVersion::get()
{
	NxU32 apiRev, descRev, branchId;
	
	int v = _physicsSDK->getInternalVersion( apiRev, descRev, branchId );
	
	return gcnew Version( (int)apiRev, (int)descRev, (int)branchId );
}
Version^ Core::SDKVersion::get()
{
	return gcnew Version( NX_SDK_VERSION_MAJOR, NX_SDK_VERSION_MINOR, NX_SDK_VERSION_BUGFIX );
}

StillDesign::PhysX::UserOutputStream^ Core::UserOutputStream::get()
{
	return _userOutputStream;
}

NxPhysicsSDK* Core::UnmanagedPointer::get()
{
	return _physicsSDK;
}