// ======================================================================== //
// Copyright 2019-2020 Ingo Wald                                            //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "Geometry.h"

namespace owl {

  struct UserGeomType : public GeomType {
    typedef std::shared_ptr<UserGeomType> SP;

    /*! any device-specific data, such as optix handles, cuda device
        pointers, etc */
    struct DeviceData : public GeomType::DeviceData {
      typedef std::shared_ptr<DeviceData> SP;

      // void writeSBTHeader(uint8_t *const sbtRecord,
      //                     Device *device,
      //                     int rayTypeID) override;
      
      void fillPGDesc(OptixProgramGroupDesc &pgDesc,
                      GeomType *gt,
                      Device *device,
                      int rayType) override;
      
      CUfunction boundsFuncKernel;
    };


    UserGeomType(Context *const context,
                 size_t varStructSize,
                 const std::vector<OWLVarDecl> &varDecls);

    virtual void setIntersectProg(int rayType,
                                  Module::SP module,
                                  const std::string &progName);
    virtual void setBoundsProg(Module::SP module,
                               const std::string &progName);

    /*! build the CUDA bounds program kernel (if bounds prog is set) */
    void buildBoundsProg();
    
    virtual std::string toString() const { return "UserGeomType"; }
    virtual std::shared_ptr<Geom> createGeom() override;

    DeviceData &getDD(int deviceID) const
    {
      assert(deviceID < deviceData.size());
      return *deviceData[deviceID]->as<DeviceData>();
    }
    DeviceData &getDD(const ll::Device *device) const { return getDD(device->ID); }
    
    /*! creates the device-specific data for this group */
    RegisteredObject::DeviceData::SP createOn(ll::Device *device) override
    { return std::make_shared<DeviceData>(); }
    
    ProgramDesc boundsProg;
    std::vector<ProgramDesc> intersectProg;
  };
  
  struct UserGeom : public Geom {
    typedef std::shared_ptr<UserGeom> SP;

    /*! any device-specific data, such as optix handles, cuda device
        pointers, etc */
    struct DeviceData : public Geom::DeviceData {
      DeviceMemory internalBufferForBoundsProgram;
    };
    /*! creates the device-specific data for this group */
    Geom::DeviceData::SP createOn(ll::Device *device) override
    { return std::make_shared<DeviceData>(); }

    inline DeviceData &getDD(ll::Device *device)
    {
      assert(device->ID < deviceData.size()); 
      return *deviceData[device->ID]->as<UserGeom::DeviceData>();
    }
                        
    UserGeomType::DeviceData &getTypeDD(int deviceID) const
    {
      return (UserGeomType::DeviceData &)type->getDD(deviceID);
    }
    UserGeomType::DeviceData &getTypeDD(Device *device) const
    {
      return (UserGeomType::DeviceData &)type->getDD(device);
    }

    UserGeom(Context *const context,
             GeomType::SP geometryType);

    virtual std::string toString() const { return "UserGeom"; }
    void setPrimCount(size_t count);
    
    /*! call a cuda kernel that computes the bounds *across* all
        primitives within this group; may only get caleld after bound
        progs have been executed */
    void computeBounds(box3f bounds[2]);

    /*! run the bounding box program for all primitives within this geometry */
    void executeBoundsProgOnPrimitives(ll::Device *device);
    
    size_t primCount = 0;
  };
  
} // ::owl
