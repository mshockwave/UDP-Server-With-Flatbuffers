// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_LEAVEGROUPREQ_FBS_MSG_H_
#define FLATBUFFERS_GENERATED_LEAVEGROUPREQ_FBS_MSG_H_

#include "flatbuffers/flatbuffers.h"

namespace fbs {
struct Session;
struct GeneralResponse;
}  // namespace fbs

namespace fbs {
namespace msg {

struct LeaveGroupRequest;

struct LeaveGroupRequest FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  const fbs::Session *session() const { return GetPointer<const fbs::Session *>(4); }
  const flatbuffers::String *group_name() const { return GetPointer<const flatbuffers::String *>(6); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, 4 /* session */) &&
           verifier.VerifyTable(session()) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, 6 /* group_name */) &&
           verifier.Verify(group_name()) &&
           verifier.EndTable();
  }
};

struct LeaveGroupRequestBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_session(flatbuffers::Offset<fbs::Session> session) { fbb_.AddOffset(4, session); }
  void add_group_name(flatbuffers::Offset<flatbuffers::String> group_name) { fbb_.AddOffset(6, group_name); }
  LeaveGroupRequestBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  LeaveGroupRequestBuilder &operator=(const LeaveGroupRequestBuilder &);
  flatbuffers::Offset<LeaveGroupRequest> Finish() {
    auto o = flatbuffers::Offset<LeaveGroupRequest>(fbb_.EndTable(start_, 2));
    return o;
  }
};

inline flatbuffers::Offset<LeaveGroupRequest> CreateLeaveGroupRequest(flatbuffers::FlatBufferBuilder &_fbb,
   flatbuffers::Offset<fbs::Session> session = 0,
   flatbuffers::Offset<flatbuffers::String> group_name = 0) {
  LeaveGroupRequestBuilder builder_(_fbb);
  builder_.add_group_name(group_name);
  builder_.add_session(session);
  return builder_.Finish();
}

inline const fbs::msg::LeaveGroupRequest *GetLeaveGroupRequest(const void *buf) { return flatbuffers::GetRoot<fbs::msg::LeaveGroupRequest>(buf); }

inline bool VerifyLeaveGroupRequestBuffer(flatbuffers::Verifier &verifier) { return verifier.VerifyBuffer<fbs::msg::LeaveGroupRequest>(); }

inline void FinishLeaveGroupRequestBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<fbs::msg::LeaveGroupRequest> root) { fbb.Finish(root); }

}  // namespace msg
}  // namespace fbs

#endif  // FLATBUFFERS_GENERATED_LEAVEGROUPREQ_FBS_MSG_H_
