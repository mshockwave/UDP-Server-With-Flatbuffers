// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_VIEWPENDINGFRIENDREQ_FBS_ACCOUNT_H_
#define FLATBUFFERS_GENERATED_VIEWPENDINGFRIENDREQ_FBS_ACCOUNT_H_

#include "flatbuffers/flatbuffers.h"

namespace fbs {
struct Session;
struct GeneralResponse;
}  // namespace fbs

namespace fbs {
namespace account {

struct ViewPendingFriendRequest;

struct ViewPendingFriendRequest FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  const fbs::Session *session() const { return GetPointer<const fbs::Session *>(4); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, 4 /* session */) &&
           verifier.VerifyTable(session()) &&
           verifier.EndTable();
  }
};

struct ViewPendingFriendRequestBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_session(flatbuffers::Offset<fbs::Session> session) { fbb_.AddOffset(4, session); }
  ViewPendingFriendRequestBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  ViewPendingFriendRequestBuilder &operator=(const ViewPendingFriendRequestBuilder &);
  flatbuffers::Offset<ViewPendingFriendRequest> Finish() {
    auto o = flatbuffers::Offset<ViewPendingFriendRequest>(fbb_.EndTable(start_, 1));
    return o;
  }
};

inline flatbuffers::Offset<ViewPendingFriendRequest> CreateViewPendingFriendRequest(flatbuffers::FlatBufferBuilder &_fbb,
   flatbuffers::Offset<fbs::Session> session = 0) {
  ViewPendingFriendRequestBuilder builder_(_fbb);
  builder_.add_session(session);
  return builder_.Finish();
}

inline const fbs::account::ViewPendingFriendRequest *GetViewPendingFriendRequest(const void *buf) { return flatbuffers::GetRoot<fbs::account::ViewPendingFriendRequest>(buf); }

inline bool VerifyViewPendingFriendRequestBuffer(flatbuffers::Verifier &verifier) { return verifier.VerifyBuffer<fbs::account::ViewPendingFriendRequest>(); }

inline void FinishViewPendingFriendRequestBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<fbs::account::ViewPendingFriendRequest> root) { fbb.Finish(root); }

}  // namespace account
}  // namespace fbs

#endif  // FLATBUFFERS_GENERATED_VIEWPENDINGFRIENDREQ_FBS_ACCOUNT_H_
