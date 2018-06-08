#include "vsa_visitor.h"
#include "llvm/Support/raw_os_ostream.h"

using namespace llvm;

namespace pcpo {

void VsaVisitor::visitBasicBlock(BasicBlock &BB){
    /// empty state represents bottom
    newState = State();
    newState.setNotBottom();
    DEBUG_OUTPUT("visitBasicBlock: entered");
    /// least upper bound with all predecessors
    for(auto pred : predecessors(&BB)){
        DEBUG_OUTPUT("visitBasicBlock: pred" << pred->getName() << " found");
        auto incoming = programPoints.find(pred);
        if(incoming != programPoints.end()){
            if(incoming->second.isBottom()) continue;
            // else state is not bottom
            DEBUG_OUTPUT("visitBasicBlock: state for" << pred->getName() << " found");
            newState.leastUpperBound(incoming->second);
        } /// else: its state is implicit bottom and lub(bottom, x) = x
    }
}

void VsaVisitor::visitTerminatorInst(TerminatorInst &I){
    /// something has changed in BB
    DEBUG_OUTPUT("visitTerminationInst: entered");
    auto currentBB = I.getParent();
    auto old = programPoints.find(currentBB);
    if(old != programPoints.end()){
        DEBUG_OUTPUT("visitTerminationInst: old state found");
        /// compute lub in place after this old state is updated
        if(!old->second.leastUpperBound(newState)){
            /// new state was old state: do not push sucessors
            return;
        }
    } else {
        DEBUG_OUTPUT("visitTerminationInst: old state not found");
        programPoints[currentBB] = newState;
    }
    
    DEBUG_OUTPUT("visitTerminationInst: state has been changed -> push successors");
    pushSuccessors(I);
}

void VsaVisitor::visitPHINode(PHINode &I){
    /// bottom
    auto bs = BoundedSet::create_bottom();
    for(Use& val:I.incoming_values()){
        //newState.getAbstractValue(val)->printOut();
            
        /// if the basic block where a value comes from is bottom,
        /// the corresponding alternative in the phi node is never taken
        /// the next 20 lines handle all the cases for that
        
        /// Check if basic block containing use is bottom
        if(Instruction::classof(val)){
            auto incomingBlock = reinterpret_cast<Instruction*>(&val)->getParent();
            
            /// block has not been visited yet -> implicit bottom
            if(programPoints.find(incomingBlock) == programPoints.end()) continue;
            
            /// explicit bottom
            if(programPoints[incomingBlock].isBottom()) continue;
        }
        
        /// if state of basic block was not bottom, include abstract value
        /// in appropriate block in lub for phi
        bs = bs->leastUpperBound(*newState.getAbstractValue(val));
    }
    
    bs->printOut();
    
    newState.put(I,bs);
}

void VsaVisitor::visitBinaryOperator(BinaryOperator &I){
    STD_OUTPUT("visited binary instruction");
}

void VsaVisitor::visitAdd(BinaryOperator &I) {
    auto ad0 = newState.getAbstractValue(I.getOperand(0));
    auto ad1 = newState.getAbstractValue(I.getOperand(1));
    
    // TODO: meaning of arguments?
    newState.put(I, ad0->add(I.getType()->getIntegerBitWidth(),*ad1, false, false));
}

void VsaVisitor::visitMul(BinaryOperator& I) {
    //auto ad0 = newState.getAbstractValue(I.getOperand(0));
    //auto ad1 = newState.getAbstractValue(I.getOperand(1));
    //newState.put(I, ad0->mul(*ad1));

}

void VsaVisitor::visitUnaryInstruction(UnaryInstruction &I){
    // todo
}

void VsaVisitor::visitInstruction(Instruction &I){
    // todo: top or exception
    STD_OUTPUT("visitInstruction: " <<I.getOpcodeName());
}

void VsaVisitor::pushSuccessors(TerminatorInst &I){
    for(auto bb : I.successors()){
        worklist.push(bb);
    }

}

}

